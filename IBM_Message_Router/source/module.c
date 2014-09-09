// **************************************************************************
//
// User module
//
// **************************************************************************

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>

#include "MQTTClient.h"

#include "module.h"
#include "regex.h"

#define QOS         	0
#define TIMEOUT     	10000L
#define PORT			1883

#define MAX_ERROR_MSG 	0x1000

#define CLIENTID_SUB    	"CID"
#define TOPIC_SUB       	"BB/#"

#define ADDRESS_PUB 		"messaging.quickstart.internetofthings.ibmcloud.com"
#define CLIENTID_PUB_BASE   "d:quickstart:wzzard:"
#define TOPIC_PUB       	"iot-2/evt/status/fmt/json"


// received signal number
volatile int got_signal = 0;

// **************************************************************************
// callback function for handling signals
static void sig_handler(int signum)
{
  // store signal number
  got_signal = signum;
}

// exit with error function
void exit_with_message(char *message) {
	printf("%s\n", message);
	syslog(LOG_ERR, message);
	syslog(LOG_NOTICE, "stopped");
	exit(-1);
}

static int compile_regex(regex_t *r, const char *regex_text)
{
	int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (status != 0) {
		char error_message[MAX_ERROR_MSG];
		regerror(status, r, error_message, MAX_ERROR_MSG);
		printf("Regex error compiling '%s': %s\n", regex_text, error_message);
		return 1;
	}
	return 0;
}

static int match_regex(regex_t *r, const char *to_match, char **result)
{
	const char *p = to_match;
	const int n_matches = 10;
	regmatch_t m[n_matches];

	while (1) {
		int i = 0;
		int nomatch = regexec(r, p, n_matches, m, 0);
		if (nomatch) {
			printf ("No more matches.\n");
			return nomatch;
		}
		for (i = 0; i < n_matches; i++) {
			int start;
			int finish;
			if (m[i].rm_so == -1) {
				break;
			}
			start = m[i].rm_so + (p - to_match);
			finish = m[i].rm_eo + (p - to_match);
			if (i == 0) {
				printf("$& is ");
			}
			else {
				printf("$%d is ", i);
			}
			printf("%.*s (bytes %d:%d)\n", (finish - start), to_match + start,
					start, finish);
			// Store desired value
			if (i != 0) {
				sprintf(*result, "%.*s", (finish - start), to_match + start);
			}
		}
		p += m[0].rm_eo;
	}
	return 0;
}


volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client_pub;
MQTTClient client_sub;
char address_pub[128];
char address_sub[128];

// Publishing client should not have confirmation of delivery as QOS is set to 0.
void delivered_pub(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

// Publishing client should not have messages arriving as it only sends them and
// the server doesn't send anything back.
int msgarrvd_pub(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Connection lost callback for publisher.
void connlost_pub(void *context, char *cause)
{
    printf("\nPublishing connection lost\n");
    printf("     cause: %s\n", cause);

	// Attempt to reconnect 5 times
	int i;
	for (i = 0; i < 5; i++) {
		printf("Attempting to reconnect to %s : Attempt %d.\n", ADDRESS_PUB, i+1);

		int rc_pub;
		MQTTClient_connectOptions conn_opts_pub = MQTTClient_connectOptions_initializer;
		conn_opts_pub.keepAliveInterval = 20;
		conn_opts_pub.cleansession = 1;

		if ((rc_pub = MQTTClient_connect(client_pub, &conn_opts_pub)) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to connect. Code : %d\n", rc_pub);
			sleep(5);
    	}
		else {
			break;
		}
	}
	exit_with_message("Failed to reconnect the publishing client.");
}


// There should be no messages being sent from the subscribing client so this
// callback should remain unused.
void delivered_sub(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

// This is the callback that forwards the Wzzard messages on to IBM. It receives
// messages from the 192.168.88.200 Spectre router and sends them back out in
// the correct format to ibm iot.
int msgarrvd_sub(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	// Alert that a message has arrived at the subscriber client.
    int i;
    char* payloadptr;

    printf("Subscribing client received message\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

	syslog(LOG_NOTICE, "Message arrived at Message Router.");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');


	// Send message out again in correct format using publisher client.
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;


	char *result = malloc(10);
	regex_t r;
	const char *pattern = "\"temp1\":([[:digit:]]+.[[:digit:]]+)";
	compile_regex(&r, pattern);
	match_regex(&r, message->payload, &result);
	// Print result from the regex calculation.
	printf("Result from regex is : %s\n", result);

	// Add in the required JSON format for ibm.
	char *fmresult = malloc(80);
	sprintf(fmresult, "{\"d\": {\"temp1\": \"%s\" }}", result);
	printf("Formatted result is now : %s\n", fmresult);

	// Send formatted message.
	pubmsg.payload = fmresult;

	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_publishMessage(client_pub, TOPIC_PUB, &pubmsg, &token);

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	free(result);
	free(fmresult);
	regfree (&r);

    return 1;
}


// Connection lost callback for subscriber.
void connlost_sub(void *context, char *cause)
{
    printf("\nSubscribing connection lost\n");
    printf("     cause: %s\n", cause);

	// Attempt to reconnect 5 times
	int i;
	for (i = 0; i < 5; i++) {
		printf("Attempting to reconnect to %s : Attempt %d.\n", address_sub, i+1);

		int rc_sub;
		MQTTClient_connectOptions conn_opts_sub = MQTTClient_connectOptions_initializer;
		conn_opts_sub.keepAliveInterval = 20;
		conn_opts_sub.cleansession = 1;

		if ((rc_sub = MQTTClient_connect(client_sub, &conn_opts_sub)) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to connect. Code : %d\n", rc_sub);
			sleep(5);
    	}
		else {
			break;
		}
	}
	exit_with_message("Failed to reconnect the subscribing client.");
}

// main function
int main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Wrong number of arguments, init script wrong!");
	exit(-1);
  }

  char clientid_pub[64];
  char clientid_sub[64];
  strcpy(clientid_pub, CLIENTID_PUB_BASE);
  strcat(clientid_pub, argv[1]);
  strcpy(clientid_sub, CLIENTID_SUB);

  sprintf(address_pub, "%s:%d", ADDRESS_PUB, PORT);
  sprintf(address_sub, "%s:%d", argv[2], PORT);

  // install signal handler
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGQUIT, sig_handler);

  // open system log
  openlog(basename(argv[0]), LOG_PID | LOG_NDELAY, LOG_DAEMON);
  // insert message to system log
  syslog(LOG_NOTICE, "started");

  // Setup publishing client (client_pub is declared globally).
  MQTTClient_connectOptions conn_opts_pub = MQTTClient_connectOptions_initializer;
  int rc_pub;

  MQTTClient_create(&client_pub, ADDRESS_PUB, clientid_pub, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  conn_opts_pub.keepAliveInterval = 20;
  conn_opts_pub.cleansession = 1;


  MQTTClient_setCallbacks(client_pub, NULL, connlost_pub, msgarrvd_pub, delivered_pub);

  if ((rc_pub = MQTTClient_connect(client_pub, &conn_opts_pub)) != MQTTCLIENT_SUCCESS) {
	exit_with_message("Failed to connect to publishing client.");
  }


  // Setup subscribing client.
  MQTTClient_connectOptions conn_opts_sub = MQTTClient_connectOptions_initializer;
  int rc_sub;

  MQTTClient_create(&client_sub, address_sub, CLIENTID_SUB, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts_sub.keepAliveInterval = 20;
  conn_opts_sub.cleansession = 1;


  MQTTClient_setCallbacks(client_sub, NULL, connlost_sub, msgarrvd_sub, delivered_sub);

  if ((rc_sub = MQTTClient_connect(client_sub, &conn_opts_sub)) != MQTTCLIENT_SUCCESS) {
    exit_with_message("Failed to connect to subscribing client."); 
  }

  printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", TOPIC_SUB, CLIENTID_SUB, QOS);
  MQTTClient_subscribe(client_sub, TOPIC_SUB, QOS);
 

  // hold this thread in a busy wait.
  while (!got_signal) { }

  // insert message to system log
  syslog(LOG_NOTICE, "stopped");

  // Cleanup subscribing client.
  MQTTClient_disconnect(client_sub, 10000);
  MQTTClient_destroy(&client_sub);

  // Cleanup publishing client.
  MQTTClient_disconnect(client_pub, 10000); 
  MQTTClient_destroy(&client_pub);

  return 0;
}	
