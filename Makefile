MAKE=make



all:    fancylogin_all fltcreate_all mingetty_all

install: fancylogin_install fltcreate_install mingetty_install documentation_install manpage_install

clean: fancylogin_clean fltcreate_clean mingetty_clean

sampleconf:
	$(MAKE) -C example install


fancylogin_all:
	$(MAKE) -C login all

fancylogin_clean:
	$(MAKE) -C login clean

fltcreate_all:
	$(MAKE) -C fltcreate all

fltcreate_clean:
	$(MAKE) -C fltcreate clean

mingetty_all:
	$(MAKE) -C mingetty all

mingetty_clean:
	$(MAKE) -C mingetty clean



fancylogin_install:
	$(MAKE) -C login install

fltcreate_install:
	$(MAKE) -C fltcreate install

mingetty_install:
	$(MAKE) -C mingetty install

documentation_install:
	$(MAKE) -C doc install_doc

manpage_install:
	$(MAKE) -C doc install_man
