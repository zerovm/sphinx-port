/*
 * main_u.c
 *
 * Released under GPL
 *
 * Copyright (C) 1998-2004 A.J. van Os
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Description:
 * The main program of 'antiword' (Unix version)
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#if defined(__dos)
#include <fcntl.h>
#include <io.h>
#endif /* __dos */
#if defined(__CYGWIN__) || defined(__CYGMING__)
#  ifdef X_LOCALE
#    include <X11/Xlocale.h>
#  else
#    include <locale.h>
#  endif
#else
#include <locale.h>
#endif /* __CYGWIN__ || __CYGMING__ */
#if defined(N_PLAT_NLM)
#if !defined(_VA_LIST)
#include "NW-only/nw_os.h"
#endif /* !_VA_LIST */
#include "getopt.h"
#endif /* N_PLAT_NLM */
#include "version.h"
#include "antiword.h"
#include "../src/zvmfileutil.h"

#define FILENAME "file.doc"
#define DEVOUTNAME "/dev/out/xmlpipecreator"


/* The name of this program */
static const char	*szTask = NULL;


static void
vUsage(void)
{
	fprintf(stderr, "\tName: %s\n", szTask);
	fprintf(stderr, "\tPurpose: "PURPOSESTRING"\n");
	fprintf(stderr, "\tAuthor: "AUTHORSTRING"\n");
	fprintf(stderr, "\tVersion: "VERSIONSTRING);
#if defined(__dos)
	fprintf(stderr, VERSIONSTRING2);
#endif /* __dos */
	fprintf(stderr, "\n");
	fprintf(stderr, "\tStatus: "STATUSSTRING"\n");
	fprintf(stderr,
		"\tUsage: %s [switches] wordfile1 [wordfile2 ...]\n", szTask);
	fprintf(stderr,
		"\tSwitches: [-f|-t|-a papersize|-p papersize|-x dtd]"
		"[-m mapping][-w #][-i #][-Ls]\n");
	fprintf(stderr, "\t\t-f formatted text output\n");
	fprintf(stderr, "\t\t-t text output (default)\n");
	fprintf(stderr, "\t\t-a <paper size name> Adobe PDF output\n");
	fprintf(stderr, "\t\t-p <paper size name> PostScript output\n");
	fprintf(stderr, "\t\t   paper size like: a4, letter or legal\n");
	fprintf(stderr, "\t\t-x <dtd> XML output\n");
	fprintf(stderr, "\t\t   like: db (DocBook)\n");
	fprintf(stderr, "\t\t-m <mapping> character mapping file\n");
	fprintf(stderr, "\t\t-w <width> in characters of text output\n");
	fprintf(stderr, "\t\t-i <level> image level (PostScript only)\n");
	fprintf(stderr, "\t\t-L use landscape mode (PostScript only)\n");
	fprintf(stderr, "\t\t-r Show removed text\n");
	fprintf(stderr, "\t\t-s Show hidden (by Word) text\n");
} /* end of vUsage */

/*
 * pStdin2TmpFile - save stdin in a temporary file
 *
 * returns: the pointer to the temporary file or NULL
 */
static FILE *
pStdin2TmpFile(long *lFilesize)
{
	FILE	*pTmpFile;
	size_t	tSize;
	BOOL	bFailure;
	UCHAR	aucBytes[BUFSIZ];

	DBG_MSG("pStdin2TmpFile");

	fail(lFilesize == NULL);

	/* Open the temporary file */
	/* FIXME if file type not of doc format (.rtf ... or other)*/
	/*char *tmpfilename = "temp.doc";*/

	/*pTmpFile = fopen (tmpfilename, "w");*/

	pTmpFile = tmpfile();
	if (pTmpFile == NULL) {
		return NULL;
	}




#if defined(__dos)
	/* Stdin must be read as a binary stream */
	setmode(fileno(stdin), O_BINARY);
#endif /* __dos */

	/* Copy stdin to the temporary file */
	*lFilesize = 0;
	bFailure = TRUE;
	for (;;) {
		tSize = fread(aucBytes, 1, sizeof(aucBytes), stdin);
		if (tSize == 0) {
			bFailure = feof(stdin) == 0;
			break;
		}
		if (fwrite(aucBytes, 1, tSize, pTmpFile) != tSize) {
			bFailure = TRUE;
			break;
		}
		*lFilesize += (long)tSize;
	}

#if defined(__dos)
	/* Switch stdin back to a text stream */
	setmode(fileno(stdin), O_TEXT);
#endif /* __dos */

	/* Deal with the result of the copy action */
	if (bFailure) {
		*lFilesize = 0;
		(void)fclose(pTmpFile);
		return NULL;
	}
	rewind(pTmpFile);
	return pTmpFile;
} /* end of pStdin2TmpFile */


/*
 * bProcessFile - process a single file
 *
 * returns: TRUE when the given file is a supported Word file, otherwise FALSE
 */

static BOOL
bProcessFile(const char *szFilename)
{
	FILE		*pFile;
	diagram_type	*pDiag;
	long		lFilesize;
	int		iWordVersion;
	BOOL		bResult;

	fail(szFilename == NULL || szFilename[0] == '\0');

	DBG_MSG(szFilename);

	if (szFilename[0] == '-' && szFilename[1] == '\0') {
		pFile = pStdin2TmpFile(&lFilesize);
		if (pFile == NULL) {
			werr(0, "I can't save the standard input to a file");
			return FALSE;
		}
	} else {
		pFile = fopen(szFilename, "rb");
		if (pFile == NULL) {
			werr(0, "I can't open '%s' for reading", szFilename);
			return FALSE;
		}

		lFilesize = lGetFilesize(szFilename);
		if (lFilesize < 0) {
			(void)fclose(pFile);
			werr(0, "I can't get the size of '%s'", szFilename);
			return FALSE;
		}
	}

	iWordVersion = iGuessVersionNumber(pFile, lFilesize);
	if (iWordVersion < 0 || iWordVersion == 3) {
		if (bIsRtfFile(pFile)) {
			werr(0, "%s is not a Word Document."
				" It is probably a Rich Text Format file",
				szFilename);
		} if (bIsWordPerfectFile(pFile)) {
			werr(0, "%s is not a Word Document."
				" It is probably a Word Perfect file",
				szFilename);
		} else {
#if defined(__dos)
			werr(0, "%s is not a Word Document or the filename"
				" is not in the 8+3 format.", szFilename);
#else
			werr(0, "%s is not a Word Document.", szFilename);
#endif /* __dos */
		}
		(void)fclose(pFile);
		return FALSE;
	}
	/* Reset any reading done during file testing */
	rewind(pFile);

	pDiag = pCreateDiagram(szTask, szFilename);
	if (pDiag == NULL) {
		(void)fclose(pFile);
		return FALSE;
	}

	bResult = bWordDecryptor(pFile, lFilesize, pDiag);
	vDestroyDiagram(pDiag);

	(void)fclose(pFile);
	return bResult;
} /* end of bProcessFile */

void fromstdintofile (void)
{
	FILE *fout;
	FILE *fin;
	fout = fopen ("temp.doc", "wb");
	fin = stdin;//fopen ("/dev/input", "rb");

	if (!fout)
	{
		printf ("*** ZVM Error save file from stdin to temp.doc\n");
		return;
	}


	char c;
	int i;
	i =0;
	while (!feof (fin))
	{
		c = getc(fin);
		if (!feof (fin))
		{
			putc (c,fout);
			i++;
		}
	}
/*
	int bread;
	int bwrite;

	long size;
	fseek(fin, 0, SEEK_END);
	size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	char buff [size + 1];

	printf ("*** ZVM size of input - %d\n", size);

	bread = fread (buff, 1, size, fin);
	bwrite = fwrite (buff, 1, size, fout);

	if (bwrite != bread)
		printf ("*** ZVM Error save from stdin\n");
*/
	printf ("*** ZVM %d bytes saved\n", i);
	//fclose (fin);
	fclose (fout);
	return;
}


void frominputtofile (void)
{
	FILE *fout;
	FILE *fin;
	fout = fopen ("temp.doc", "wb");
	fin = fopen ("/dev/input", "rb");

	if (!fout)
	{
		printf ("*** ZVM Error save file from stdin to temp.doc\n");
		return;
	}

	int bread;
	int bwrite;

	long size;
	fseek(fin, 0, SEEK_END);
	size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	char buff [size + 1];

	printf ("size of input - %d\n", size);

	bread = fread (buff, 1, size, fin);
	bwrite = fwrite (buff, 1, size, fout);

	if (bwrite != bread)
		printf ("*** ZVM Error save from input\n");

	printf ("*** ZVM %d bytes saved\n", bwrite);
	fclose (fin);
	fclose (fout);
	return;
}

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


int
wmain(int argc, char **argv)
{
	options_type	tOptions;
	const char	*szWordfile;
	int	iFirst, iIndex, iGoodCount;
	BOOL	bUsage, bMultiple, bUseTXT, bUseXML;


	//fromstdintofile ();
	//frominputtofile ();

	if (argc <= 0) {
		return EXIT_FAILURE;
	}

	szTask = szBasename(argv[0]);

	if (argc <= 1) {
		iFirst = 1;
		bUsage = TRUE;
	} else {
		iFirst = iReadOptions(argc, argv);
		bUsage = iFirst <= 0;
	}
	if (bUsage) {
		vUsage();
		return iFirst < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

#if defined(N_PLAT_NLM) && !defined(_VA_LIST)
	nwinit();
#endif /* N_PLAT_NLM && !_VA_LIST */

	vGetOptions(&tOptions);

#if !defined(__dos)
	if (is_locale_utf8()) {
#if defined(__STDC_ISO_10646__)
		/*
		 * If the user wants UTF-8 and the envirionment variables
		 * support UTF-8, than set the locale accordingly
		 */
		if (tOptions.eEncoding == encoding_utf_8) {
			if (setlocale(LC_CTYPE, "") == NULL) {
				werr(1, "Can't set the UTF-8 locale! "
					"Check LANG, LC_CTYPE, LC_ALL.");
			}
			DBG_MSG("The UTF-8 locale has been set");
		} else {
			(void)setlocale(LC_CTYPE, "C");
		}
#endif /* __STDC_ISO_10646__ */
	} else {
		if (setlocale(LC_CTYPE, "") == NULL) {
			werr(0, "Can't set the locale! Will use defaults");
			(void)setlocale(LC_CTYPE, "C");
		}
		DBG_MSG("The locale has been set");
	}
#endif /* !__dos */

	bMultiple = argc - iFirst > 1;
	bUseTXT = tOptions.eConversionType == conversion_text ||
		tOptions.eConversionType == conversion_fmt_text;
	bUseXML = tOptions.eConversionType == conversion_xml;
	iGoodCount = 0;

#if defined(__dos)
	if (tOptions.eConversionType == conversion_pdf) {
		/* PDF must be written as a binary stream */
		setmode(fileno(stdout), O_BINARY);
	}
#endif /* __dos */

	if (bUseXML) {
		fprintf(stdout,
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	"<!DOCTYPE %s PUBLIC \"-//OASIS//DTD DocBook XML V4.1.2//EN\"\n"
	"\t\"http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd\">\n",
		bMultiple ? "set" : "book");
		if (bMultiple) {
			fprintf(stdout, "<set>\n");
		}
	}

	for (iIndex = iFirst; iIndex < argc; iIndex++) {
		if (bMultiple && bUseTXT) {
			szWordfile = szBasename(argv[iIndex]);
			fprintf(stdout, "::::::::::::::\n");
			fprintf(stdout, "%s\n", szWordfile);
			fprintf(stdout, "::::::::::::::\n");
		}
		if (bProcessFile(argv[iIndex])) {
			iGoodCount++;
		}
	}

	if (bMultiple && bUseXML) {
		fprintf(stdout, "</set>\n");
	}

	DBG_DEC(iGoodCount);

	if (iGoodCount <= 0)
		return 1;

	fflush (NULL);
/*
	FILE *f;
	FILE *fout;
	f = fopen ("temp.txt", "r");
	fout = fopen ("/dev/out/xmlpipecreator", "w");


	if (!f)
	{
		printf ("*** ZVM Error open temp.txt file\n");
		printheader (fout);
		printfooter (fout);
		return 1;
	}
	if (!fout)
	{
		printf ("*** ZVM Error open output device (xmlpipeceator)\n");
		return 1;
	}
	int c;
	int lastc;
	printheader (fout);
	printheader (stdout);
	while (!feof (f))
	{
		c = (char)getc (f);
		if ((c != EOF) && (isalnum(c) || (isspace (c)&& !isspace(lastc))))
		{
			putc (c, fout);
			putc (c, stdout);
		}
		else
			putc(' ', fout);
		lastc = c;
	}
	printfooter (fout);
	printfooter (stdout);
	fclose (fout);
	fclose (f);
*/
	return iGoodCount <= 0 ? EXIT_FAILURE : EXIT_SUCCESS;
} /* end of main */

int main (int argc, char *argv[])
{

    struct filemap fmap;

    char *filename;
    char *chname; //
    int fdout;

    char *path = "/dev/in";
    //char *path = "/home/volodymyr/git/sphinx-port/docxextract/1";
    char *prefix = "";
    //char *prefix = "/home/volodymyr/temp";
    long totalbyteswrite2text, byteswrite2text;

  	DIR *dir;
	struct dirent *entry;


    byteswrite2text = 0;
    totalbyteswrite2text = 0;
//    fdout = open ("/home/volodymyr/temp/xmlpipe", O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);

    fdout = open (DEVOUTNAME, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
    if (fdout < 0 )
    {
    	printf ("*** ZVM Error open % output device\n", DEVOUTNAME);
    	return 1;
    }

    /* create temp.doc file in "/" directory
	if (mkdir (prefix, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR) != 0)
	{
		printf ("*** ZVM Error create dir %s\n", prefix);
		return 1;
	}
	*/
	dir = opendir(path);
	if (dir ==0)
	{
		printf ("*** ZVM Error read dir %s, %d\n", path, dir);
		return 1;
	}
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
			char *tmpdocfilename = "temp.doc";
			char *tmptxtfilename = TEMP_FILE_FOR_EXTRACTED_TEXT; // tempextracted.txt
			int fdin;
			chname = malloc (strlen(path) + strlen (entry->d_name) + 2);
			sprintf (chname, "%s/%s", path, entry->d_name);
			printf ("start chname = %s\n", chname);
			fmap = getfilefromchannel(chname, prefix);
			/*
			 *
			 * extract text from DOC
			 *
			 */
			int tmpsize, ftmp;
			ftmp = open (fmap.tempfilename, O_RDONLY);

			tmpsize = getfilesize_fd(ftmp, NULL, 0);
			printf ("tmp file name = %s, size = %d, detected size = %d\n", fmap.tempfilename, fmap.realfilesize, tmpsize);
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

			ftmp = open (tmpdocfilename, O_RDONLY);
			tmpsize = getfilesize_fd(ftmp, NULL, 0);
			printf ("tmp file name after rename = %s, size = %d, detected size = %d\n", tmpdocfilename, fmap.realfilesize, tmpsize);
			close (ftmp);

			if (wmain (argc, argv) != 0)
			{
				printf ("*** Error extract %s \n", tmpdocfilename);
				continue;
			}
			fdin = open (tmptxtfilename, O_RDONLY);
			if (fdin <=0 )
			{
				printf ("*** Error fdin = %d \n", fdin);
				continue;
			}

			txtbufflen = getfilesize_fd(fdin,NULL,0);
			printf ("txtbufflen = %d, (%s)\n", txtbufflen, tmptxtfilename);


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

			int tempwritebytes2channel;
			if (filteredbufflen > 0)
			{
				//printf ("*****filteredbufflen %d\n", filteredbufflen);
				//for (temp = 0; temp < filteredbufflen; temp++)
				//	putchar (filteredbuff[temp]);
				tempwritebytes2channel = puttext2channel (filteredbuff, filteredbufflen, fmap.realfilename, fdout);
			}
			else
				continue;

		}
	}

	close (fdout);
	printf ("doc - all OK!\n");

	return 0;
}
