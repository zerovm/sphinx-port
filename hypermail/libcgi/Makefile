# Generated automatically from Makefile.in by configure.
# CGI virdoc library makefile

#WNOERROR=-Werror
#WARNINGS=$(WNOERROR) -ansi -pedantic -Wall -Wtraditional -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Dlint
 
# You shouldn't have to edit anything else.
CC=x86_64-nacl-gcc $(WARNINGS)
INSTALL=../install-sh
AUX_LIBS=
RANLIB=x86_64-nacl-ranlib
RM=rm
LINT=lint

SRCS=form_ent.c get_cgi_info.c main.c syn_mime.c syn_url.c mcode.c\
        form_tags.c strops.c html.c

.c.o:
	$(CC) -c $(CFLAGS) $(AUX_CFLAGS) $(INF_INCS) $<

ALL = libcgi.a

%.o: %.c
	$(CC) -c $(CFLAGS) $(AUX_CFLAGS) $(INF_INCS) $<

all: $(ALL)

libcgi.a: form_ent.o get_cgi_info.o main.o syn_mime.o syn_url.o mcode.o\
	form_tags.o strops.o html.o
	ar r $@ $?
	${RANLIB} $@

install: $(ALL)

clean:
	-$(RM) -f *.o *~ *.a

clobber: clean
distclean: clean

insight: clean
	$(MAKE) CC="insight"

lint:
	$(LINT) $(LINTFLAGS) $(SRCS) 2>&1 | tee lint.out

