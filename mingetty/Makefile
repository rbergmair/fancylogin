CC=cc
CFLAGS=
LDFLAGS=
SBINDIR=/sbin

all: 
	$(CC) mingetty.c $(CFLAGS) $(LDFLAGS) -o mingetty

clean:
	rm -f *.o mingetty

install: all
	rm -f /sbin/mingetty.fancylogin
	cp mingetty $(SBINDIR)/mingetty.fancylogin
