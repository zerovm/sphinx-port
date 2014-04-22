
LIBS = -lz -lrt -lm -lexpat -lpthread -ldl

LIBS_PDF = ../../zxpdf-3.03/goo/libGoo.a ../../zxpdf-3.03/fofi/libfofi.a ../../zxpdf-3.03/splash/libsplash.a 

CC = x86_64-nacl-gcc
CXX = x86_64-nacl-g++
CFLAGS = -g -O0

EXE = .nexe

OBJS = zvmfileutil.o other.o other_.o xmlpipecreator.o filesender.o main_indexer.o opt.o filesender_.o

OBJS_MAIN_INDEXER = main_indexer.o opt.o other_.o xmlpipecreator_.o filesender_.o zvmfileutil.o fileutil.o lists.o

OBJS_SINGLE_OBJECT_INDEXER = single_object_indexer.o fileutil.o lists.o xml_util.o
LIBS_SINGLE_OBJECT_INDEXER =  ../../docxextract/libdocx.a ../../src/libindexer.a -lz 
#../../antiword-0.37/libdoc.a

all:  other$(EXE) xmlpipecreator$(EXE) filesender$(EXE) main_indexer$(EXE) single_object_indexer$(EXE)

filesender$(EXE): zvmfileutil.o filesender.o filesender_.o 
	$(CC) filesender.o zvmfileutil.o filesender_.o -o filesender$(EXE) $(LIBS) $(CFLAGS)

other$(EXE): zvmfileutil.o other.o other_.o
	$(CC) other.o zvmfileutil.o other_.o -o other$(EXE) $(LIBS) $(CFLAGS)

xmlpipecreator$(EXE): zvmfileutil.o xmlpipecreator.o xmlpipecreator_.o
	$(CC) xmlpipecreator.o xmlpipecreator_.o zvmfileutil.o -o xmlpipecreator$(EXE) $(LIBS) $(CFLAGS)

xmlpipecreator_.o:
	$(CC) -c xmlpipecreator_.c $(LIBS) $(CFLAGS)

other.o:
	$(CC) -c other.c $(LIBS) $(CFLAGS)

other_.o:
	$(CC) -c other_.c $(LIBS) $(CFLAGS)

filesender.o:
	$(CC) -c filesender.c $(LIBS) $(CFLAGS)

filesender_.o:
	$(CC) -c filesender_.c $(LIBS) $(CFLAGS)
	
main_indexer.o:
	$(CC) -c main_indexer.c $(LIBS) $(CFLAGS)

zvmfileutil.o:
	$(CC) -c zvmfileutil.c $(LIBS) $(CFLAGS)

opt.o:
	$(CC) -o opt.o -c opt.c $(LIBS) $(CFLAGS)
	
single_object_indexer.o: single_object_indexer.c
	$(CC) -c -o $@ $^ $(LIBS) $(CFLAGS) 

fileutil.o: fileutil.c
	$(CC) -c -o $@ $^ $(LIBS) $(CFLAGS)

lists.o: lists.c
	$(CC) -c -o $@ $^ $(LIBS) $(CFLAGS)

xml_util.o: xml_util.c
	$(CC) -c -o $@ $^ $(LIBS) $(CFLAGS)

main_indexer$(EXE): $(OBJS_MAIN_INDEXER) ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a
	$(CC) -o main_indexer$(EXE) $(OBJS_MAIN_INDEXER) ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a ../../src/libsphinx.a $(LIBS) $(LIBS_PDF)   

single_object_indexer$(EXE): $(OBJS_SINGLE_OBJECT_INDEXER)
	$(CC) -o  $@ $(OBJS_SINGLE_OBJECT_INDEXER) $(CFLAGS) ../../zxpdf-3.03/xpdf/libpdf.a $(LIBS_SINGLE_OBJECT_INDEXER) $(LIBS_PDF) $(LIBS) ../../hypermail/src/hypermail.a -liconv -lpcre -lm ../../catdoc-0.94.4/src/catdoc.a


clean:
	rm -f *.o *$(EXE)
	
