CC=x86_64-nacl-gcc
#CFLAGS=

UNZ_OBJS = docxtotext.o unzip.o ioapi.o zip.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c 

all: docxtotext

docxtotext:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS) -lz

clean:
	/bin/rm -f *.o *~ docxtotext
