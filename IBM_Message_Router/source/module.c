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

#define QOS         	0
#define TIMEOUT     	10000L

#define MAX_ERROR_MSG 	0x1000

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

	// Attempt to reconnect 3 times
	int i;
	for (i = 0; i < 3; i++) {
		printf("Attempting to reconnect to %s : Attempt %d.\n", address_pub, i+1);

		int rc_pub;
		MQTTClient_connectOptions conn_opts_pub = MQTTClient_connectOptions_initializer;
		conn_opts_pub.keepAliveInterval = 20;
		conn_opts_pub.cleansession = 1;

		if ((rc_pub = MQTTClient_connect(client_pub, &conn_opts_pub)) != MQTTCLIENT_SUCCESS) {
        	printf("Failed to connect. Code : %d\n", rc_pub);
			sleep(2);
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
	syslog(LOG_NOTICE, "Message arrived at Message Router.");

	// Send message out again in correct format using publisher client.
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;

	int i;
	char *payloadptr;
	char *resptr;
	char resulth[128];
	char *result[128];

	payloadptr = message->payload;
	resptr = resulth;
	for(i = 0; i < message->payloadlen; i++) {
		*resptr = *payloadptr++;
		resptr++;
	}
    *resptr = '\0';

	sprintf(result, "{\"d\":%s}", resulth);
	printf("%s\n", result);

	pubmsg.payload = result;
	pubmsg.payloadlen = strlen(pubmsg.payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_publishMessage(client_pub, "iot-2/evt/status/fmt/json", &pubmsg, &token);

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

    return 1;
}


// Connection lost callback for subscriber.
void connlost_sub(void *context, char *cause)
{
    printf("\nSubscribing connection lost\n");
    printf("     cause: %s\n", cause);

	// Attempt to reconnect 3 times
	int i;
	for (i = 0; i < 3; i++) {
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
//arg1 is devId, arg2 is devIp
//arg3 is mode, arg4 is orgid, arg5 is dtypeid, arg6 is token
int main(int argc, char *argv[])
{
  printf("%s, %s, %s, %s, %s, %s \n", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

  int mode = atoi(argv[3]);
  char clientid_pub[64];

  if (mode == 1) {
	sprintf(clientid_pub, "d:%s:%s:%s", argv[4], argv[5], argv[1]);
	sprintf(address_pub, "%s.%s:%d", argv[4],
			"messaging.internetofthings.ibmcloud.com", 1883);
  } else {
	sprintf(clientid_pub, "d:quickstart:wzzard:%s", argv[1]);
    sprintf(address_pub, "%s:%d", "messaging.quickstart.internetofthings.ibmcloud.com", 1883);
  }
  sprintf(address_sub, "%s:%d", argv[2], 1883);

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

  MQTTClient_create(&client_pub, address_pub, clientid_pub, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  conn_opts_pub.keepAliveInterval = 20;
  conn_opts_pub.cleansession = 1;
  
  if (mode == 1) {
    conn_opts_pub.username = "use-token-auth";
    conn_opts_pub.password = argv[6];
  }


  MQTTClient_setCallbacks(client_pub, NULL, connlost_pub, msgarrvd_pub, delivered_pub);

  if ((rc_pub = MQTTClient_connect(client_pub, &conn_opts_pub)) != MQTTCLIENT_SUCCESS) {
	exit_with_message("Failed to connect to publishing client.");
  }


  // Setup subscribing client.
  MQTTClient_connectOptions conn_opts_sub = MQTTClient_connectOptions_initializer;
  int rc_sub;

  MQTTClient_create(&client_sub, address_sub, "CID", MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts_sub.keepAliveInterval = 20;
  conn_opts_sub.cleansession = 1;

  MQTTClient_setCallbacks(client_sub, NULL, connlost_sub, msgarrvd_sub, delivered_sub);

  if ((rc_sub = MQTTClient_connect(client_sub, &conn_opts_sub)) != MQTTCLIENT_SUCCESS) {
    exit_with_message("Failed to connect to subscribing client."); 
  }

  printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", "BB/#", "CID", QOS);
  MQTTClient_subscribe(client_sub, "BB/#", QOS);
 

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
