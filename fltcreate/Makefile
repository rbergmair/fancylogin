CC=gcc
LIBTERMCAP=-ltermcap
LIBREADLINE=-lreadline
USRBINDIR=/usr/bin



all:
	$(CC) fltcreate.c $(LIBTERMCAP) $(LIBREADLINE) -o fltcreate

clean:
	rm -f fltcreate

install: all
	rm -f $(USRBINDIR)/fltcreate
	cp fltcreate $(USRBINDIR)
