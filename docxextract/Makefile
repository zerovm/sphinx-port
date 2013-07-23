#CC=gcc
CC=x86_64-nacl-gcc
CFLAGS=-g2 -O0 

UNZ_OBJS = docxtotext.o unzip.o ioapi.o zip.o ../src/zvmfileutil.c

.c.o:
	$(CC) -c $(CFLAGS) $*.c 

all: docxtotext

docxtotext:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS) -lz

clean:
	/bin/rm -f *.o *~ docxtotext
