CC=gcc
CFLAGS=-O2 -Wall -ansi
LDFLAGS=
LIBS=/usr/lib
LIBNCURSES=$(LIBS)/libncurses.so
LIBCRYPT=$(LIBS)/libcrypt.so
BINDIR=/bin

MODULES=authenticated.o fancy.o login.o login_permissions.o log_message.o system_logon.o emergency.o signal_handle.o

all:    $(MODULES) link

authenticated.o:
	$(CC) $(CFLAGS) -c authenticated.c

emergency.o:
	$(CC) $(CFLAGS) -c emergency.c

fancy.o:
	$(CC) $(CFLAGS) -c fancy.c

login.o: 
	$(CC) $(CFLAGS) -c login.c

login_permissions.o:
	$(CC) $(CFLAGS) -c login_permissions.c

log_message.o:
	$(CC) $(CFLAGS) -c log_message.c

signal_handle.o:
	$(CC) $(CFLAGS) -c signal_handle.c

system_logon.o:
	$(CC) $(CFLAGS) -c system_logon.c

link:
	$(CC) $(MODULES) $(LIBNCURSES) $(LIBCRYPT) -o fancylogin

install:
	rm -f $(BINDIR)/fancylogin
	cp fancylogin $(BINDIR)

sampleconf:
	cp ../doc/sample/signon.* /etc

clean:
	rm -f $(MODULES)
	rm -f fancylogin
