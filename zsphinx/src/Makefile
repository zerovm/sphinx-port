

LIBS = -lz -lrt -lm -lexpat -lpthread -ldl

LIBS_PDF = ../../zxpdf-3.03/goo/libGoo.a ../../zxpdf-3.03/fofi/libfofi.a ../../zxpdf-3.03/splash/libsplash.a 

CC = x86_64-nacl-gcc
CXX = x86_64-nacl-g++

EXE = .nexe

OBJS = zvmfileutil.o other.o xmlpipecreator.o filesender.o main_indexer.o


all:  other$(EXE) xmlpipecreator$(EXE) filesender$(EXE) main_indexer$(EXE)

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
	
main_indexer.o:
	$(CC) -o main_indexer.o -c main_indexer.c 
	
main_indexer$(EXE): main_indexer.o ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a
	$(CC) -o main_indexer$(EXE) main_indexer.o ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a ../../src/libsphinx.a $(LIBS) $(LIBS_PDF)   

clean:
	rm -f $(OBJS) *$(EXE)
