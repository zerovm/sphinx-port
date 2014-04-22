#CC=gcc
CC=x86_64-nacl-gcc
CFLAGS=-g2 -O2

AR = ar cru
RANLIB = x86_64-nacl-ranlib

UNZ_OBJS = docxtotext.o docxtotext_.o unzip.o ioapi.o zip.o zvmfileutil.o fileutil.o lists.o 
LIB_OBJS = docxtotext_.o unzip.o ioapi.o zip.o 
ZVM_SRC = ../zsphinx/src/zvmfileutil.c  


LIBDOCX = libdocx.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c 

all: docxtotext $(LIBDOCX)

docxtotext:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS) -lz

zvmfileutil.o:
	$(CC) $(CFLAGS) -c $(ZVM_SRC)
	
fileutil.o: 
	$(CC) $(CFLAGS) -c ../zsphinx/src/fileutil.c 

lists.o:
	$(CC) $(CFLAGS) -c ../zsphinx/src/lists.c
	
libdocx.a: $(LIB_OBJS)
	rm -f libdocx.a
	$(AR) $(LIBDOCX) $(LIB_OBJS) 
	$(RANLIB) $(LIBDOCX) 

clean:
	/bin/rm -f *.o *~ docxtotext
	rm -f $(LIBDOCX)
