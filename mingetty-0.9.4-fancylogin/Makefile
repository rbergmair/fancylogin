CC=cc
CFLAGS=
LDFLAGS=
SBINDIR=/sbin

all: 
	$(CC) mingetty.c $(CFLAGS) $(LDFLAGS) -o mingetty

clean:
	rm -f mingetty

install:
	rm -f $(SBINDIR)/mingetty.fancylogin
	cp mingetty $(SBINDIR)/mingetty.fancylogin
