CC = arm-linux-gcc
CFLAGS = -O2 -Wall -W -Wmissing-declarations -Wshadow
CPPFLAGS = -I../libs/common -I./MQTTClient -I../libs
LDFLAGS = -s -L../libs/common -L./MQTTClient -L../libs
LDLIBS = -lcommon -ldl -lssl -lcrypto -lpthread


MODULE_EXE = module
MODULE_OBJ = module.o
MQTT_OBJ = MQTTClient/Clients.o MQTTClient/Heap.o MQTTClient/LinkedList.o MQTTClient/Log.o MQTTClient/Messages.o MQTTClient/MQTTClient.o MQTTClient/MQTTPacket.o MQTTClient/MQTTPacketOut.o MQTTClient/MQTTPersistence.o MQTTClient/MQTTPersistenceDefault.o MQTTClient/MQTTProtocolClient.o MQTTClient/MQTTProtocolOut.o MQTTClient/SocketBuffer.o MQTTClient/Socket.o MQTTClient/SSLSocket.o MQTTClient/StackTrace.o MQTTClient/Thread.o MQTTClient/Tree.o MQTTClient/utf-8.o

MODULE_CGI_EXE = module_cgi
MODULE_CGI_OBJ = module_cgi.o module_cfg.o

all: $(MODULE_EXE) $(MODULE_CGI_EXE)

$(MODULE_EXE): $(MODULE_OBJ) $(MQTT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(MODULE_CGI_EXE): $(MODULE_CGI_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(MODULE_EXE) $(MODULE_CGI_EXE) *.o

install:
	install -d $(DESTDIR)/bin/
	install -d $(DESTDIR)/www/
	install -m 755 $(MODULE_EXE) $(DESTDIR)/bin/IBM_MRouterd
	install -m 755 $(MODULE_CGI_EXE) $(DESTDIR)/bin/cgi
	ln -s ../bin/cgi $(DESTDIR)/www/index.cgi
	ln -s ../bin/cgi $(DESTDIR)/www/restart.cgi
	ln -s ../bin/cgi $(DESTDIR)/www/set.cgi
	ln -s ../bin/cgi $(DESTDIR)/www/slog.cgi
	ln -s /etc/htpasswd $(DESTDIR)/www/.htpasswd
