CC=gcc
CFLAGS= -ggdb -Wall -O2 -ansi
LIBCRYPT=-lcrypt
BINDIR=/bin
DOCS=/usr/doc/fancylogin-0.99.7-alpha2

MODULES=authenticated.o login.o login_permissions.o log_message.o system_logon.o emergency.o signal_handle.o after_login.o md5crypt.o fancy.o

all:	$(MODULES) link

authenticated.o: authenticated.c
	$(CC) $(CFLAGS) -c authenticated.c

after_login.o: after_login.c
	$(CC) $(CFLAGS) -c after_login.c

emergency.o: emergency.c
	$(CC) $(CFLAGS) -c emergency.c

fancy.o: fancy.c
	$(CC) $(CFLAGS) -c fancy.c

login.o: login.c
	$(CC) $(CFLAGS) -c login.c

login_permissions.o: login_permissions.c
	$(CC) $(CFLAGS) -c login_permissions.c

log_message.o: log_message.c
	$(CC) $(CFLAGS) -c log_message.c

md5crypt.o: md5crypt.c
	$(CC) $(CFLAGS) -c md5crypt.c

signal_handle.o: signal_handle.c
	$(CC) $(CFLAGS) -c signal_handle.c

system_logon.o: system_logon.c
	$(CC) $(CFLAGS) -c system_logon.c

link:
	$(CC) $(MODULES) $(LDFLAGS) $(LIBNCURSES) $(LIBCRYPT) $(PAMMODULES) -o fancylogin

clean:
	rm -f $(MODULES) fancylogin

install:
	rm -f $(BINDIR)/fancylogin
	cp fancylogin $(BINDIR)
