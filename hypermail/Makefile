# Generated automatically from Makefile.in by configure.
#
# Makefile for hypermail package.
#



prefix=/usr/local
exec_prefix=${prefix}

# This is where you want hypermail to be installed
bindir=${exec_prefix}/bin

# This is where the man page goes
mandir=${prefix}/man

srcdir=.

# This is where the HTML documentation goes
htmldir=/usr/local/apache/htdocs/hypermail

# This is where your CGI programs live
cgidir=/usr/local/apache/cgi-bin

INSTALL_PROG=install-sh

#WNOERROR=-Werror
#WARNINGS=$(WNOERROR) -ansi -pedantic -Wall -Wtraditional -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Dlint
 
# Compiler to use
CC=x86_64-nacl-gcc 
CFLAGS=-g -O2  -Ipcre -DTRIO_MINIMAL $(WARNINGS)
LIBS= -lnsl
LDFLAGS= -Lpcre

all:   hypermail support 

hypermail:
	@cd src; $(MAKE) all CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		cgidir="$(cgidir)" bindir="$(bindir)" LIBS="$(LIBS)"

support:   
	@cd archive; $(MAKE) all CC="$(CC)" CFLAGS="$(CFLAGS)" 

install: 
	@cd src; $(MAKE) install CC="$(CC)" CFLAGS="$(CFLAGS)" \
		cgidir="$(cgidir)" bindir="$(bindir)" LIBS="$(LIBS)"
	@cd docs; $(MAKE) install CC="$(CC)" CFLAGS="$(CFLAGS)" \
		$(MAKEFLAGS) mandir="$(mandir)" htmldir="$(htmldir)"
	@cd archive; $(MAKE) install CC="$(CC)" CFLAGS="$(CFLAGS)" \
		bindir="$(bindir)"

uninstall: 
	@cd src; $(MAKE) uninstall cgidir="$(cgidir)" bindir="$(bindir)" 
	@cd docs; $(MAKE) uninstall mandir="$(mandir)" htmldir="$(htmldir)"
	@cd archive; $(MAKE) uninstall bindir="$(bindir)"

lint:	
	@cd src; $(MAKE) lint 
	@cd archive; $(MAKE) lint 

clean:
	@cd src; $(MAKE) clean 
	@cd docs; $(MAKE) clean 
	@cd archive; $(MAKE) clean 
	rm -f lint.out

clobber: clean
	rm -f config.h
	rm -f config.cache
	rm -f config.log
	rm -f config.status
	rm -f archive/Makefile
	rm -f docs/Makefile
	rm -f libcgi/Makefile
	rm -f src/Makefile
	rm -f src/defaults.h
	rm -f tests/testhm
	rm -rf tests/testdir
	rm -rf tests/mail-archive
	rm -f Makefile

distclean: clobber
	rm -f tests/testmail

tgz:
	@(dir=`pwd`;name=`basename $$dir`;echo Creates $$name.tar.gz; cd .. ; \
	tar -cf $$name.tar \
           `cat $$name/FILES | grep -v "^#" | sed "s:^:$$name/:g"` ; \
	gzip $$name.tar ; chmod a+r $$name.tar.gz ; mv $$name.tar.gz $$name/)

