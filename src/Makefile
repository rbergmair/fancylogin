CC=cc
CFLAGS=
LDFLAGS=
LIBS=/usr/lib
LIBNCURSES=$(LIBS)/libncurses.so
LIBCRYPT=$(LIBS)/libcrypt.so
BINDIR=/bin

all: fancy.o login.o link install sampleconf

try: fancy.o login.o link

fancy.o:
	$(CC) -c fancy.c

login.o: 
	$(CC) -c login.c

link:
	$(CC) fancy.o login.o $(LIBNCURSES) $(LIBCRYPT) -o fancylogin

install:
	cp fancylogin $(BINDIR)

sampleconf:
	cp ../doc/sample/* /etc

clean:
	rm fancy.o
	rm login.o
	rm fancylogin 
      
