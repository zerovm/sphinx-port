#========================================================================
#
# Xpdf Makefile
#
# Copyright 1996-2003 Glyph & Cog, LLC
#
#========================================================================

LIBS = -lz

CC = x86_64-nacl-gcc

EXE = .nexe

all:  other$(EXE) xmlpipecreator$(EXE) filesender$(EXE)

filesender$(EXE): zvmfileutil.o filesender.o
	$(CC) filesender.c zvmfileutil.c -o filesender$(EXE) $(LIBS)

other$(EXE): zvmfileutil.o other.o
	$(CC) other.c zvmfileutil.c -o other$(EXE) $(LIBS)

xmlpipecreator$(EXE): zvmfileutil.o xmlpipecreator.o
	$(CC) xmlpipecreator.c zvmfileutil.c -o xmlpipecreator$(EXE) $(LIBS)

other.o:
	$(CC) -o other.o other.c zvmfileutil.c -lz

filesender.o:
	$(CC) -o filesender.o filesender.c zvmfileutil.c -lz

OBJS = zvmfileutil.o other.o xmlpipecreator.o filesender.o



clean:
	rm -f $(OBJS) *$(EXE)
