CC = arm-linux-gcc
CFLAGS = -O2 -Wall -W -Wmissing-declarations -Wshadow
CPPFLAGS = -I../common -I./MQTTClient
LDFLAGS = -s -L../common -L../libs
LDLIBS = -lcommon -lpaho-mqtt3c -lpthread

MODULE_EXE = module
MODULE_OBJ = module.o

MODULE_CGI_EXE = module_cgi
MODULE_CGI_OBJ = module_cgi.o

all: $(MODULE_EXE) $(MODULE_CGI_EXE)

$(MODULE_EXE): $(MODULE_OBJ)
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
	ln -s ../bin/cgi $(DESTDIR)/www/slog.cgi
	ln -s /etc/htpasswd $(DESTDIR)/www/.htpasswd
