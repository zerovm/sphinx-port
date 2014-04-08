

LIBS = -lz -lrt -lm -lexpat -lpthread -ldl

LIBS_PDF = ../../zxpdf-3.03/goo/libGoo.a ../../zxpdf-3.03/fofi/libfofi.a ../../zxpdf-3.03/splash/libsplash.a 

CC = x86_64-nacl-gcc
CXX = x86_64-nacl-g++

EXE = .nexe

OBJS = zvmfileutil.o other.o other_.o xmlpipecreator.o filesender.o main_indexer.o opt.o filesender_.o

OBJS_MAIN_INDEXER = main_indexer.o opt.o other_.o xmlpipecreator_.o filesender_.o zvmfileutil.o

OBJS_SINGLE_OBJECT_INDEXER = single_object_indexer.o fileutil.o lists.o xml_util.o
LIBS_SINGLE_OBJECT_INDEXER = ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../src/libindexer.a -lz
#

all:  other$(EXE) xmlpipecreator$(EXE) filesender$(EXE) main_indexer$(EXE)

filesender$(EXE): zvmfileutil.o filesender.o filesender_.o 
	$(CC) filesender.o zvmfileutil.o filesender_.o -o filesender$(EXE) $(LIBS)

other$(EXE): zvmfileutil.o other.o other_.o
	$(CC) other.o zvmfileutil.o other_.o -o other$(EXE) $(LIBS)

xmlpipecreator$(EXE): zvmfileutil.o xmlpipecreator.o xmlpipecreator_.o
	$(CC) xmlpipecreator.o xmlpipecreator_.o zvmfileutil.o -o xmlpipecreator$(EXE) $(LIBS)

xmlpipecreator_.o:
	$(CC) -c xmlpipecreator_.c $(LIBS)

other.o:
	$(CC) -c other.c $(LIBS)

other_.o:
	$(CC) -c other_.c $(LIBS)

filesender.o:
	$(CC) -c filesender.c $(LIBS)

filesender_.o:
	$(CC) -c filesender_.c $(LIBS)
	
main_indexer.o:
	$(CC) -c main_indexer.c $(LIBS)

zvmfileutil.o:
	$(CC) -c zvmfileutil.c $(LIBS)

opt.o:
	$(CC) -o opt.o -c opt.c $(LIBS)
	
single_object_indexer.o: single_object_indexer.c
	$(CC) -c -o $@ $^ $(LIBS) 

fileutil.o: fileutil.c
	$(CC) -c -o $@ $^ $(LIBS) 

lists.o: lists.c
	$(CC) -c -o $@ $^ $(LIBS) 

xml_util.o: xml_util.c
	$(CC) -c -o $@ $^ $(LIBS) 

main_indexer$(EXE): $(OBJS_MAIN_INDEXER) ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a
	$(CC) -o main_indexer$(EXE) $(OBJS_MAIN_INDEXER) ../../antiword-0.37/libdoc.a ../../docxextract/libdocx.a ../../zxpdf-3.03/xpdf/libpdf.a ../../src/libindexer.a ../../src/libsphinx.a $(LIBS) $(LIBS_PDF)   

single_object_indexer: $(OBJS_SINGLE_OBJECT_INDEXER)
	$(CC) -o $@ $(OBJS_SINGLE_OBJECT_INDEXER) ../../zxpdf-3.03/xpdf/libpdf.a $(LIBS_SINGLE_OBJECT_INDEXER) $(LIBS_PDF) $(LIBS) 


clean:
	rm -f *.o *$(EXE)
	
