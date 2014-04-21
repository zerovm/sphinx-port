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
#include <unistd.h>

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
#include "../zsphinx/src/zvmfileutil.h"

#define FILENAME "file.doc"


/* The name of this program */
static const char	*szTask = NULL;

extern char **environ;


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
bProcessFile(const char *szFilename, const char *fname_save)
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

	pDiag = pCreateDiagram(szTask, szFilename, fname_save);
	if (pDiag == NULL) {
		(void)fclose(pFile);
		return FALSE;
	}

	bResult = bWordDecryptor(pFile, lFilesize, pDiag);
	vDestroyDiagram(pDiag);

	(void)fclose(pFile);
	return bResult;
} /* end of bProcessFile */

int doc_to_text (char *input_file, char *file_save)
{
	char *argv[] = { "doc", input_file };
	int argc = sizeof ( argv ) / sizeof (char *);
	return wmain ( argc, argv, file_save );
}

int
wmain(int argc, char **argv, char *file_save)
{
	options_type	tOptions;
	const char	*szWordfile;
	int	iFirst, iIndex, iGoodCount;
	BOOL	bUsage, bMultiple, bUseTXT, bUseXML;

	printf ( "GLOBAL_ANTIWORD_DIR=%s\n", GLOBAL_ANTIWORD_DIR);
	printf ( "ANTIWORD_DIR=%s\n", ANTIWORD_DIR);
	printf ( "FONTNAMES_FILE=%s\n", FONTNAMES_FILE);


	if (argc <= 0) {
		return EXIT_FAILURE;
	}

	szTask = szBasename(argv[0]);

	argc = 1;

	if (argc <= 1) {
		iFirst = 1;
		bUsage = TRUE;
	} else {
		iFirst = iReadOptions((int)argc, (char **)argv);
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
		if (bProcessFile(argv[iIndex], file_save)) {
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
	return iGoodCount <= 0 ? EXIT_FAILURE : EXIT_SUCCESS;
} /* end of main */


int textconv_doc (char * path, char *d_name, char * prefix, int bTextSearchMode, char * filteredbuff, char * buff, int fdout, size_t tStart, size_t tEnd, int argc, char *argv[])
{
	long txtbufflen;
	long filteredbufflen;
	char *chname = NULL;
	char *tmpdocfilename = DOC_TEMP_DOC_FILE_NAME;
	char *tmptxtfilename = DOC_TEMP_FILE_FOR_EXTRACTED_TEXT; // tempextracted.txt
	int fdin;
	struct filemap fmap;
	const char *PINFO = "PATH_INFO";

	if (bTextSearchMode == 0 )
	{
		chname = malloc (strlen(path) + strlen (d_name) + 2);
		sprintf (chname, "%s/%s", path, d_name);
		LOG_ZVM (ZLOGTIT, "incoming channel", "s", chname, 1);
		fmap = extractorfromfilesender(chname, prefix);
	}
	else
	{
		sprintf (fmap.tempfilename, "%s/temp.tmp", prefix);
		fmap.realfilesize = SaveFileFromInput (fmap.tempfilename, environ, dev_stdin);
		fmap.json = generateJson(environ);
		if (getenv (PINFO) != NULL)
			sprintf (fmap.realfilename, "%s", getenv (PINFO));

	}

	fileTypeInfo_t fti;
	LOG_ZVM (ZLOGTIT, "real file name", "s", fmap.realfilename, 1);
	fti = checkMAxFileSize ();

	if (fti.sExt == NULL)
		return 0;

	if (strncmp ( fti.sExt , "doc", 3) != 0)
		return 0;

	/*
	 *
	 * extract text from DOC
	 *
	 */

	int tmpsize, ftmp;
	ftmp = open (fmap.tempfilename, O_RDONLY);
	tmpsize = getfilesize_fd(ftmp, NULL, 0);
	close (ftmp);

	LOG_ZVM (ZLOGTIT, "check size of temporary saved file", "d", tmpsize, 2);

	if (fti.bSaveFile == 1)
	{
		if (fmap.realfilesize <= 0)
		{
			printf ("*** Error fmap.realfilesize = %ld\n", fmap.realfilesize);
			return 0;
		}
		if (rename (fmap.tempfilename, tmpdocfilename))
		{
			printf ("*** Error rename %s to %s \n", fmap.tempfilename, tmpdocfilename);
			return 0;
		}

		int extractorretcode = 0;
		if ((extractorretcode = wmain (argc, argv, DOC_TEMP_FILE_FOR_EXTRACTED_TEXT)) != 0)
		{
			LOG_ZVM (ZLOGTIT, "extractor return code", "d", extractorretcode, 1);
			return 0;
		}
		LOG_ZVM (ZLOGTIT, "extractor return code", "d", extractorretcode, 1);

		fdin = open (tmptxtfilename, O_RDONLY);
		if (fdin <=0 )
		{
			printf ("*** Error fdin = %d \n", fdin);
			return 0;
		}

		txtbufflen = getfilesize_fd(fdin,NULL,0);
		LOG_ZVM (ZLOGTIT, "text file size", "ld", txtbufflen, 1);

		if (txtbufflen <0 )
		{
			printf ("*** Error txtbufflen = %ld \n", txtbufflen);
			return 0;
		}
		buff = (char *) malloc (txtbufflen);
		filteredbuff = (char *) malloc (txtbufflen);
		int bread;
		bread = read (fdin, buff, txtbufflen);
		close (fdin);

		filteredbufflen = getfilteredbuffer (buff, txtbufflen, filteredbuff);
		LOG_ZVM (ZLOGTIT, "filtered buffer length", "ld", filteredbufflen, 1);
	}


	int tempwritebytes2channel;

	if (filteredbufflen > 0 || fti.bSaveFile == 1)
	{
		if (bTextSearchMode == 0)
		{
			//
			if (Mode == single_operation)
			{
				LOG_ZVM (ZLOGTIT, "Mode", "s", "single operation", 1);
				WRITE_TO_LOCAL_FILE_EXTRACTOR;
			}
			else
			{
				LOG_ZVM (ZLOGTIT, "Mode", "s", "single operation", 1);
				tempwritebytes2channel = puttext2channel (filteredbuff, filteredbufflen, fmap.realfilename, fmap.json, fdout);
			}
			LOG_ZVM (ZLOGTIT, "bytes write to output channel by current document", "d", tempwritebytes2channel, 1);
		}
		else
		{
			PrintSnippet (filteredbuff, fmap.realfilename,  tStart, tEnd);
		}
	}
	else
		return 0;
	return tempwritebytes2channel;
}

int doc_main (int argc, char *argv[])
{

	char *serversoft = getenv ("SERVER_SOFTWARE");
	LOG_SERVER_SOFT;
	LOG_NODE_NAME;

    int fdout;

    char *path = DOC_DEVICE_IN;
    char *prefix = DOC_PREFIX;

    long totalbyteswrite2text, byteswrite2text;

  	DIR *dir;
	struct dirent *entry;

    byteswrite2text = 0;
    totalbyteswrite2text = 0;

    char devoutname [strlen (DEVOUTNAME) * 2];
	////////////////////////////////////////
	// program options
	////////////////////////////////////////
	struct p_options popt;
    ////////////////////////////////////////
    // end program options
    ////////////////////////////////////////

	popt = getOptions(argc, argv);


	if (popt.bTextSearchMode == 1)
	{
		argc = 2;
	}

	int tempwritebytes2channel;

    if (popt.bTextSearchMode == 0)
    {
        if (popt.bToOutput == 1)
        	sprintf (devoutname, "%s", "/dev/output");
        else
        	sprintf (devoutname, "%s", DEVOUTNAME);
    	if (Mode == single_operation)
        	sprintf (devoutname, "%s", EXTRACTOR_SINGLE_MODE_OUTPUT_FILE);

        fdout = open (devoutname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
		if (fdout < 0 )
		{
			printf ("*** ZVM Error open %s output device\n", devoutname);
			return 1;
		}
		LOG_ZVM (ZLOGTIT, "output channel", "s", devoutname, 1);

		if (Mode == cluster)
		{
			dir = opendir(path);
			if (dir ==0)
			{
				printf ("*** ZVM Error read dir %s\n", path);
				return 1;
			}
			LOG_ZVM (ZLOGTIT, "incoming channels dir", "s", path, 1);
			while((entry = readdir(dir)))
			{
				int filteredbufflen;
				filteredbufflen =0;
				if(entry->d_type != DT_DIR && (strcmp (entry->d_name, "input")) != 0)
				{
					char *filteredbuff = NULL; 	// buffer for filtered text extracted from file of any format
					char *buff = NULL; 			// temp buffer for readed data from txt file.
					tempwritebytes2channel = textconv_doc (path, entry->d_name, prefix, popt.bTextSearchMode, filteredbuff, buff, fdout, popt.tStart, popt.tEnd, argc, argv);

				}
			}
		}
		else
		{
			char *filteredbuff = NULL; 	// buffer for filtered text extracted from file of any format
			char *buff = NULL; 			// temp buffer for readed data from txt file.
			tempwritebytes2channel = textconv_doc ("", FILESENDER_SINGLE_MODE_OUTPUT_FILE, prefix, popt.bTextSearchMode, filteredbuff, buff, fdout, popt.tStart, popt.tEnd, argc, argv);
		}
	}
	else
	{
		char *filteredbuff = NULL; 	// buffer for filtered text extracted from file of any format
		char *buff = NULL; 			// temp buffer for readed data from txt file.
		tempwritebytes2channel = textconv_doc (path, entry->d_name, prefix, popt.bTextSearchMode, filteredbuff, buff, fdout, popt.tStart, popt.tEnd, argc, argv);
	}
	close (fdout);
	LOG_ZVM (ZLOGTIT, "total bytes write to output channel", "ld", totalbyteswrite2text, 1);
	LOG_ZVM (ZLOGTIT, "OK!", "s", "", 1);
	return 0;
}


