Summary: themeable login program for Linux
Name: fancylogin
Version: 0.99.7
Release: 1
Copyright: GPL
Group: System/Login
Source: fancylogin-0.99.7.tar.gz
Packager: Andreas Krennmair <a.krennmair@aon.at>
%description
fancylogin is one of the most powerful login programs available
for Linux. fancylogin can do everything your old login program 
can do, e.g. handling shadowed passwd files, user-time-terminal/
network-verification as done with HP-UX login, etc. It adds a
lot of capabilities for logging logins and support for themes
to control the login's look.

%prep
%setup

%build
make

%install
make install
make sampleconf

%files

%doc doc/compiling.html doc/configuration.html doc/credits.html doc/docbook.css doc/faq.html doc/index.html doc/license.html doc/obtaining.html doc/stylesheet-images/caution.gif doc/stylesheet-images/home.gif doc/stylesheet-images/important.gif doc/stylesheet-images/next.gif doc/stylesheet-images/note.gif doc/stylesheet-images/prev.gif doc/stylesheet-images/tip.gif doc/stylesheet-images/toc-blank.gif doc/stylesheet-images/toc-minus.gif doc/stylesheet-images/toc-plus.gif doc/stylesheet-images/up.gif doc/stylesheet-images/warning.gif

/bin/fancylogin
/sbin/mingetty.fancylogin
/etc/default.flt
/usr/man/man1/fancylogin.1
