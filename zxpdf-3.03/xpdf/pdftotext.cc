//========================================================================
//
// pdftotext.cc
//
// Copyright 1997-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parseargs.h"
#include "GString.h"
#include "gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "TextOutputDev.h"
#include "CharTypes.h"
#include "UnicodeMap.h"
#include "Error.h"
#include "config.h"
#include "../../src/zvmfileutil.h"


static void printInfoString(FILE *f, Dict *infoDict, const char *key,
			    const char *text1, const char *text2,
			    UnicodeMap *uMap);
static void printInfoDate(FILE *f, Dict *infoDict, const char *key,
			  const char *fmt);

static int firstPage = 1;
static int lastPage = 0;
static GBool physLayout = gFalse;
static double fixedPitch = 0;
static GBool rawOrder = gFalse;
static GBool htmlMeta = gFalse;
static char textEncName[128] = "";
static char textEOL[16] = "";
static GBool noPageBreaks = gFalse;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool quiet = gFalse;
static char cfgFileName[256] = "";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-f",       argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",       argInt,      &lastPage,      0,
   "last page to convert"},
  {"-layout",  argFlag,     &physLayout,    0,
   "maintain original physical layout"},
  {"-fixed",   argFP,       &fixedPitch,    0,
   "assume fixed-pitch (or tabular) text"},
  {"-raw",     argFlag,     &rawOrder,      0,
   "keep strings in content stream order"},
  {"-htmlmeta", argFlag,   &htmlMeta,       0,
   "generate a simple HTML file, including the meta information"},
  {"-enc",     argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-eol",     argString,   textEOL,        sizeof(textEOL),
   "output end-of-line convention (unix, dos, or mac)"},
  {"-nopgbrk", argFlag,     &noPageBreaks,  0,
   "don't insert page breaks between pages"},
  {"-opw",     argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",     argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-q",       argFlag,     &quiet,         0,
   "don't print any messages or errors"},
  {"-cfg",     argString,   cfgFileName,    sizeof(cfgFileName),
   "configuration file to use in place of .xpdfrc"},
  {"-v",       argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",       argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",    argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",       argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};


/*
void mylistdir (char *path)
{
  	DIR *dir;
	struct dirent *entry;
	//struct stat sb;
	char statcheck [1024];
	char newpath[1024];
	char newpathf[1024];
	char extfile [1024];
	dir = opendir(path);
	int len, lennew, lennewlast;
	if(dir == 0)
	{
		return;
	}
//	printf ("* %s", path);
	while(entry = readdir(dir))
	{
		//printf ("%s/%s\n",path, entry->d_name);
		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				strcpy (newpath, path);
				len = strlen (newpath);
				if (newpath [len-1] != '/')
					strcat (newpath, "/");
				strcat (newpath, entry->d_name);
				mylistdir (newpath);
			}
		}
		else
		{

		}

	}
	closedir(dir);
}
*/
void printheader (FILE *f)
{
	char *externalfilename = getenv("fname");
	//printf ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // заголовок XML файла, генерируется в xmlpipecreator
	//printf ("<sphinx:document id=\"%d\">\n", docID); // заголовок документа в потоке, генерируется в xmlpipecreator
	fprintf (f, "<filename>%s</filename>\n", externalfilename); //
	fprintf (f, "<content>\n");

}

void printfooter (FILE *f)
{
	  fprintf (f, "</content>\n");
	  fprintf (f, "</sphinx:document>\n \n");
}

void wprintheader (FILE *f)
{
	char *externalfilename = getenv("fname");
	//printf ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // заголовок XML файла, генерируется в xmlpipecreator
	//printf ("<sphinx:document id=\"%d\">\n", docID); // заголовок документа в потоке, генерируется в xmlpipecreator
	fwprintf (f, L"<filename>%s</filename>\n", externalfilename); //
	fwprintf (f, L"<content>\n");

}

void wprintfooter (FILE *f)
{
	  fwprintf (f, L"</content>\n");
	  fwprintf (f, L"</sphinx:document>\n \n");
}

#undef ZVMDEBUG

int wmain(int argc, char *argv[]) {
  PDFDoc *doc;
  GString *fileName;
  GString *textFileName;
  GString *ownerPW, *userPW;
  TextOutputDev *textOut;
  FILE *f;
  UnicodeMap *uMap;
  Object info;
  GBool ok;
  char *p;
  int c;
  int exitCode;

  exitCode = 99;

/*
  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftotext version %s\n", xpdfVersion);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftotext", "<PDF-file> [<text-file>]", argDesc);
    }
    goto err0;
  }
*/
//#ifdef ZVMDEBUG
//#endif
  // copy from stdin incoming pdf file to temp file
  fileName = new GString("temp.pdf");
  if (fixedPitch) {
    physLayout = gTrue;
  }
/*
#ifndef ZVMSTDIN
  FILE *tempPDF;
  printf ("*** start saving PDF from stdin to temp PDF file \n");
  tempPDF = fopen (fileName->getCString(), "w");
  while ((c = getchar()) != EOF)
	  fprintf (tempPDF, "%c", c);
  fclose (tempPDF);
  printf ("*** save PDF from stdin to temp PDF file complete \n");
#endif
*/
  // read config file
  globalParams = new GlobalParams(cfgFileName);
  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }
  if (textEOL[0]) {
    if (!globalParams->setTextEOL(textEOL)) {
      fprintf(stderr, "Bad '-eol' value on command line\n");
    }
  }
  if (noPageBreaks) {
    globalParams->setTextPageBreaks(gFalse);
  }
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

  // get mapping to output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    error(errConfig, -1, "Couldn't get text encoding");
    delete fileName;
    goto err1;
  }

  // open PDF file
  if (ownerPassword[0] != '\001') {
    ownerPW = new GString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0] != '\001') {
    userPW = new GString(userPassword);
  } else {
    userPW = NULL;
  }
#ifdef ZVMDEBUG
  printf ("***read %s file \n", fileName->getCString());
#endif
  doc = new PDFDoc(fileName, ownerPW, userPW);
#ifdef ZVMDEBUG
  printf("***read %s file complete! \n", fileName->getCString());
#endif
/*
  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    exitCode = 1;
    goto err2;
  }
*/
  // check for copy permission
/*
  if (!doc->okToCopy()) {
    error(errNotAllowed, -1,
	  "Copying of text from this document is not allowed.");
    exitCode = 3;
    goto err2;
  }
*/
/*
  // construct text file name
  if (argc == 3) {
    textFileName = new GString(argv[2]);
  } else {
    p = fileName->getCString() + fileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
      textFileName = new GString(fileName->getCString(),
				 fileName->getLength() - 4);
    } else {
      textFileName = fileName->copy();
    }
    textFileName->append(htmlMeta ? ".html" : ".txt");
  }
*/

  textFileName = new GString("temp.txt");

  // get page range
  if (firstPage < 1) {
    firstPage = 1;
  }

  if (lastPage < 1 || lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }
#ifdef ZVMDEBUG
  printf ("*** getNumPaghes OK!!! \n");
#endif
/*
  // write HTML header
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "wb"))) {
	error(errIO, -1, "Couldn't open text file '{0:t}'", textFileName);
	exitCode = 2;
	goto err3;
      }
    }
    fputs("<html>\n", f);
    fputs("<head>\n", f);
    doc->getDocInfo(&info);
    if (info.isDict()) {
      printInfoString(f, info.getDict(), "Title", "<title>", "</title>\n",
		      uMap);
      printInfoString(f, info.getDict(), "Subject",
		      "<meta name=\"Subject\" content=\"", "\">\n", uMap);
      printInfoString(f, info.getDict(), "Keywords",
		      "<meta name=\"Keywords\" content=\"", "\">\n", uMap);
      printInfoString(f, info.getDict(), "Author",
		      "<meta name=\"Author\" content=\"", "\">\n", uMap);
      printInfoString(f, info.getDict(), "Creator",
		      "<meta name=\"Creator\" content=\"", "\">\n", uMap);
      printInfoString(f, info.getDict(), "Producer",
		      "<meta name=\"Producer\" content=\"", "\">\n", uMap);
      printInfoDate(f, info.getDict(), "CreationDate",
		    "<meta name=\"CreationDate\" content=\"%s\">\n");
      printInfoDate(f, info.getDict(), "LastModifiedDate",
		    "<meta name=\"ModDate\" content=\"%s\">\n");
    }
    info.free();
    fputs("</head>\n", f);
    fputs("<body>\n", f);
    fputs("<pre>\n", f);
    if (f != stdout) {
      fclose(f);
    }
  }
*/
  // write text file
  textOut = new TextOutputDev(textFileName->getCString(),
			      physLayout, fixedPitch, rawOrder, htmlMeta);
  if (textOut->isOk()) {
    doc->displayPages(textOut, firstPage, lastPage, 72, 72, 0,
		      gFalse, gTrue, gFalse);
  } else {
    delete textOut;
    exitCode = 2;
    goto err3;
  }
  delete textOut;
/*
  // write end of HTML file
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "ab"))) {
	error(errIO, -1, "Couldn't open text file '{0:t}'", textFileName);
	exitCode = 2;
	goto err3;
      }
    }
    fputs("</pre>\n", f);
    fputs("</body>\n", f);
    fputs("</html>\n", f);
    if (f != stdout) {
      fclose(f);
    }
  }
*/
  exitCode = 0;
  // write temp text to output device

  char *oldlocale;
  oldlocale = setlocale (LC_ALL, "ru_RU.UTF-8");
#ifdef ZVMDEBUG
  printf ("*** set locale to %s\n", oldlocale);
#endif
/*
  FILE *devfile;
  FILE *tempfile;
  devfile = fopen ("/dev/out/xmlpipecreator", "w");
  tempfile = fopen ( textFileName->getCString()  , "r");
  if (!devfile)
	  printf ("!*** error open dev output file\n");
  if (!tempfile)
	  printf ("!*** error open temp.txt file\n");
  int cc, cc1;
  cc = 0;
  cc1 = 0;
  wchar_t wc, lastsymbol;
  wprintheader (devfile);
  while ((wc = (wchar_t) fgetwc(tempfile))!= WEOF)
  {
	  cc1++;
	  if (iswalnum(wc) || (iswspace (wc) && !iswspace (lastsymbol)))
	  {
		  cc++;
		  fputwc (wc, devfile);
	  }
	  else
		  fputwc (' ', devfile);
	  lastsymbol = wc;
  }
  wprintfooter (devfile);
  //fflush (stdout);
  fclose (devfile);
  fclose (tempfile);

  printf ("***extracting %d symbols filtering %d symbols\n", cc1, cc);//, getenv ["fname"]);
*/
#ifdef ZVMDEBUG
  mylistdir ("/");
#endif
  // clean up
 err3:
  delete textFileName;
 err2:
  delete doc;
  uMap->decRefCnt(	);
 err1:
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}

int main (int argc, char *argv[])
{

	char *serversoft = getenv ("SERVER_SOFTWARE");

	LOG_SERVER_SOFT;
	LOG_NODE_NAME;

	struct filemap fmap;

    char *filename;
    char *chname; //
    int fdout;

    char *path = (char*)PDF_DEVICE_IN;
    char *prefix = (char*)PDF_PREFIX;

    long totalbyteswrite2text, byteswrite2text;

  	DIR *dir;
	struct dirent *entry;

    byteswrite2text = 0;
    totalbyteswrite2text = 0;

    fdout = open (DEVOUTNAME, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
    if (fdout < 0 )
    {
    	printf ("*** ZVM Error open % output device\n", DEVOUTNAME);
    	return 1;
    }
	LOG_ZVM ("***ZVMLog", "output channel", "s", DEVOUTNAME, 1);

	dir = opendir(path);
	if (dir ==0)
	{
		printf ("*** ZVM Error read dir %s, %d\n", path, dir);
		return 1;
	}
	LOG_ZVM ("***ZVMLog", "incoming channels dir", "s", path, 1);
	while(entry = readdir(dir))
	{
		int temp;
		int filteredbufflen;
		filteredbufflen =0;
		if(entry->d_type != DT_DIR && (strcmp (entry->d_name, "input")) != 0)
		{
			char *filteredbuff; // buffer for filtered text extracted from file of any format
			char *buff; // temp buffer fot readed data trom txt file.
			long txtbufflen;
			long filteredbufflen;
			char *tmpdocfilename = (char*)PDF_TEMP_PDF_FILE_NAME;
			char *tmptxtfilename = (char*)PDF_TEMP_TXT_FILE_NAME;//TEMP_FILE_FOR_EXTRACTED_TEXT; // tempextracted.txt
			int fdin;
			chname = (char *) malloc (strlen(path) + strlen (entry->d_name) + 2);
			sprintf (chname, "%s/%s", path, entry->d_name);
			LOG_ZVM ("***ZVMLog", "incoming channel", "s", chname, 1);
			fmap = extractorfromfilesender(chname, prefix);
			/*
			 *
			 * extract text from PDF
			 *
			 */
			int tmpsize, ftmp;
			ftmp = open (fmap.tempfilename, O_RDONLY);

			tmpsize = getfilesize_fd(ftmp, NULL, 0);
			LOG_ZVM ("***ZVMLog", "check size of temporary saved file", "d", tmpsize, 2);
			close (ftmp);
			if (fmap.realfilesize <= 0)
			{
				printf ("*** Error fmap.realfilesize = %d\n", fmap.realfilesize);
				continue;
			}
			if (rename (fmap.tempfilename, tmpdocfilename))
			{
				printf ("*** Error rename %s to %s \n", fmap.tempfilename, tmpdocfilename);
				continue;
			}

			int extractorretcode = 0;
			if (wmain (argc, argv) != 0)
			{
				LOG_ZVM ("***ZVMLog", "extractor return code", "d", extractorretcode, 1);
				printf ("*** Error extract %s \n", tmpdocfilename);
				continue;
			}
			LOG_ZVM ("***ZVMLog", "extractor return code", "d", extractorretcode, 1);

			fdin = open (tmptxtfilename, O_RDONLY);
			if (fdin <=0 )
			{
				printf ("*** Error fdin = %d \n", fdin);
				continue;
			}

			txtbufflen = getfilesize_fd(fdin,NULL,0);
			LOG_ZVM ("***ZVMLog", "text file size", "ld", txtbufflen, 1);


			if (txtbufflen <0 )
			{
				printf ("*** Error txtbufflen = %d \n", txtbufflen);
				continue;
			}
			buff = (char *) malloc (txtbufflen);
			filteredbuff = (char *) malloc (txtbufflen);
			int bread;
			bread = read (fdin, buff, txtbufflen);
			close (fdin);

			filteredbufflen = getfilteredbuffer (buff, txtbufflen, filteredbuff);
			LOG_ZVM ("***ZVMLog", "filtered buffer length", "ld", filteredbufflen, 1);

			int tempwritebytes2channel;
			if (filteredbufflen > 0)
			{
				tempwritebytes2channel = puttext2channel (filteredbuff, filteredbufflen, fmap.realfilename, fmap.json, fdout);
				LOG_ZVM ("***ZVMLog", "bytes write to output channel by current document", "d", tempwritebytes2channel, 1);
				totalbyteswrite2text += tempwritebytes2channel;
			}
			else
				continue;

		}
	}

	close (fdout);
	LOG_ZVM ("***ZVMLog", "total bytes write to output channel", "ld", totalbyteswrite2text, 1);
	LOG_ZVM ("***ZVMLog", "OK!", "s", "", 0);

	return 0;
}

static void printInfoString(FILE *f, Dict *infoDict, const char *key,
			    const char *text1, const char *text2,
			    UnicodeMap *uMap) {
  Object obj;
  GString *s1;
  GBool isUnicode;
  Unicode u;
  char buf[8];
  int i, n;

  if (infoDict->lookup(key, &obj)->isString()) {
    fputs(text1, f);
    s1 = obj.getString();
    if ((s1->getChar(0) & 0xff) == 0xfe &&
	(s1->getChar(1) & 0xff) == 0xff) {
      isUnicode = gTrue;
      i = 2;
    } else {
      isUnicode = gFalse;
      i = 0;
    }
    while (i < obj.getString()->getLength()) {
      if (isUnicode) {
	u = ((s1->getChar(i) & 0xff) << 8) |
	    (s1->getChar(i+1) & 0xff);
	i += 2;
      } else {
	u = s1->getChar(i) & 0xff;
	++i;
      }
      n = uMap->mapUnicode(u, buf, sizeof(buf));
      fwrite(buf, 1, n, f);
    }
    fputs(text2, f);
  }
  obj.free();
}

static void printInfoDate(FILE *f, Dict *infoDict, const char *key,
			  const char *fmt) {
  Object obj;
  char *s;

  if (infoDict->lookup(key, &obj)->isString()) {
    s = obj.getString()->getCString();
    if (s[0] == 'D' && s[1] == ':') {
      s += 2;
    }
    fprintf(f, fmt, s);
  }
  obj.free();
}
