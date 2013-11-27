/*
 * zvmfileutil.c
 *
 *  Created on: Jul 8, 2013
 *      Author: Varchuk Volodymyr
 */

#include <stdio.h>
#include <string.h>
#include "zvmfileutil.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <zlib.h>


extern char **environ;

char *blank_attr_list[] = {
		"PATH_INFO", // objectname - PATH_INFO str
		"TIMESTAMP", // HTTP_X_TIMESTAMP; X_TIMESTAMP; TIMESTAMP timestamp
		"CONTENT_LENGTH", // CONTENT_LENGTH int
		"meta" // JSON
};

char *blank_attr_list_types[] = {
		"string", // objectname  - PATH_INFO str
		"timestamp", // HTTP_X_TIMESTAMP; X_TIMESTAMP; TIMESTAMP timestamp
		"int", // CONTENT_LENGTH int
		"json" // JSON
};

char *blank_field_list[] = {
		"content", // internal document text
		"metatags", // all metatags words only
};
char *blank_field_list_types[] = {
		"string", // internal document text
		"string", // all metatags words only
};

int blank_field_count = sizeof (blank_field_list) / sizeof (char *);
int blank_attr_count = sizeof (blank_attr_list) / sizeof (char *);




/*
 * reverse null-terminated string
*/
void reverse (char s[])
{
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/*
 * returns extension of file if has a "." in name return file name if has not "." in his name
*/
void getext (const char *fname, char *ext)
{
	int len = strlen (fname);
	int i;
	i = 0;
	if (len == 0)
		return;

	while ((fname [len] != '.') && (fname [len] != '/') && len != 0)
	{
		if ((fname [len] != '\n') && (fname [len] != '\0'))
			ext[i++] = fname [len--];
		else
			len--;
	}
	ext[i] = '\0';
	reverse (ext);
	return;
}

void sendConfigOK ()
{
	int fd = open (CHECK_INDEXER_IND_DEV_NAME, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	int bytewrite = 0;

	if (fd < 0)
	{
		return;
	}

	if ((bytewrite = write(fd, "OK", 2)) != 2)
	{
		printf ("***Error write to CHECK_INDEXER_IND_DEV_NAME\n");
		close (fd);
		return;
	}
	LOG_ZVM ("***ZVMLog", "send OK to xmlpipe", "s", "OK", 1);
	close (fd);
	return;
}


/*
 * return size of file in bytes by file descriptor if sizebyfilename=0
 *
 * and
 *
 * return size of file in bytes by file name if sizebyfilename=1
*/
unsigned long getfilesize_fd (int fd, char *filename, int sizebyfilename)
{
	if (sizebyfilename == 1)
		fd = open (filename, O_RDONLY);
	if (fd < 0)
	{
		printf ("*** ZVM Error get filesize wrong file descriptor %d\n", fd);
		return -1;
	}

	long fsize;
	fsize = lseek (fd, 0, SEEK_END);
	if (fsize < 0)
	{
		printf ("*** ZVM Error get filesize for file descriptor %d\n", fd);
		if (sizebyfilename == 1)
			close(fd);
		return -1;
	}
	int temp = lseek (fd, 0, SEEK_SET);
	if (temp < 0)
		printf ("*** ZVM Warning! I can't SEEK_SET execute for file descriptor %d\n", fd);

	if (sizebyfilename == 1)
		close(fd);

	return fsize;
}

int getZVMLogLevel ()
{
	int iLocalZVMLogLevel = 0;
	if (getenv("ZVM_LogLevel") != NULL)
	{
		iLocalZVMLogLevel = atoi (getenv("ZVM_LogLevel"));
	}
	else
		iLocalZVMLogLevel = 0;
	//printf (" !!! ");
	return iLocalZVMLogLevel;
}

/*
* write to indexer file document header for sphinx xml format
*/
void printdochead (int fd, unsigned long docID)
{
	char str[50]; // 50 - stock
	int bwrite;

	sprintf (str, "<sphinx:document id=\"%lu\">\n", docID);
	bwrite = write (fd, str, strlen (str));

/*
	sprintf (str, "<filename><![CDATA[%s]]></filename>\n", realfilename);
	bwrite = write (fd, str, strlen (str));
*/
	return;
}

void printjson (int fd, char *json)
{
	// <meta> </meta>;
	char str [strlen (json) + 50]; // 50 - stock
	int bwrite = 0;

	sprintf (str, "<meta><![CDATA[%s]]></meta>\n", json);
	bwrite = write (fd, str, strlen (str));

	return;
}

int write_XML_Elemet (int fd, char *elemeNtname, char *element)
{
	int bwrite = 0;
	int strSize = 50;// stock
	if (element != NULL)
		strSize += strlen(element) * 2;
	if (elemeNtname != NULL)
		strSize += strlen(elemeNtname);
	else
	{
		printf ("*** ZVM Error. Wrong element name\n");
		return 0;
	}
	char *str = (char*) malloc (sizeof (char) * strSize);
	sprintf (str, "<%s><![CDATA[%s]]></%s>\n", elemeNtname, element, elemeNtname);
	bwrite = write (fd, str, strlen (str));
	return bwrite;
}

/*
* write to indexer file documet header for sphinx xml format
*/
void printdocfooter (int fd)
{
	const char *str = "\n</sphinx:document>\n \n";
	int bwrite;
	bwrite = write (fd, str, strlen (str));
	return;
}

/*
 * get value from "key":"value"
*/

char * getValue (const char * pKeyVal)
{
	char *pValue = NULL;

	char *pPosColon = strstr (pKeyVal, ":") + 1;

	pValue = (char *) malloc ( strlen (pPosColon));
	strncpy (pValue, pPosColon + 1, strlen (pPosColon) - 1);
	pValue[strlen (pPosColon) - 2] = '\0';

	return pValue;
}

/*
 * get key from "key":"value" string
*/

char * getKey (const char *pKeyVal)
{
	char *pKey;

	if (pKeyVal == NULL)
		return NULL;

	char *pPosColon1 = strstr (pKeyVal, "\"") + 1;
	if (pPosColon1 == NULL)
		return NULL;

	char *pPosColon2 = strstr (pPosColon1, "\"");
	if (pPosColon2 == NULL)
		return NULL;

	pKey = (char *) malloc (sizeof (char * ) * (pPosColon2 - pPosColon1 + 5));

	strncpy (pKey, pPosColon1, pPosColon2 - pPosColon1);
	pKey [pPosColon2 - pPosColon1] = '\0';

	return pKey;
}

// filtering json
char *generateMetaWords (char *json)
{
	char *metawords;
	if (json == NULL)
	{
		printf ("*** ZVM Error. Wrong json\n");
		return NULL;
	}
	metawords = (char *) malloc (sizeof (char) * strlen (json));
	size_t a = 0;
	for (a = 0; a < strlen (json); a++)
	{
		if (json [a] == ',')
			metawords[a] = ';';
		else if (json [a] != '"' && json [a] != ':' && json [a] != '{' && json [a] != '}' )
		{
			metawords[a] = json [a];
		}
		else
		{
			metawords[a] = ' ';
		}
		if (json [a] == '}')
			metawords[a] = '\0';
		if (!isalnum (json [a]) )
			json [a] = ' ';
	}

	return metawords;
}

/*
 * get text data from network in format size of text, real filename, text data
*/
int getdatafromchannel (int fd, char *chname, int docID, struct field_list fl)
{

/*
	char *blank_attr_list[] = {
			"PATH_INFO", // objectname  - PATH_INFO str
			"TIMESTAMP", // HTTP_X_TIMESTAMP; X_TIMESTAMP; TIMESTAMP timestamp
			"CONTENT_LENGTH", // CONTENT_LENGTH int
			"meta" // JSON
	};
	char *blank_attr_list_types[] = {
			"string", // objectname  - PATH_INFO str
			"timestamp", // HTTP_X_TIMESTAMP; X_TIMESTAMP; TIMESTAMP timestamp
			"int", // CONTENT_LENGTH int
			"json" // JSON
	};


	char *blank_field_list[] = {
			"content", // internal document text
			"metatags", // all metatags words only
	};
	char *blank_field_list_types[] = {
			"string", // internal document text
			"string", // all metatags words only
	};


	const int blank_field_count = sizeof (blank_field_list) / sizeof (char *);
	const int blank_attr_count = sizeof (blank_attr_list) / sizeof (char *);
*/



	int fdin;
	char c, lastc;
	fdin = open (chname, O_RDONLY);
	if (fdin < 0)
	{
		printf ("*** ZVM Error open %s channel descriptor %d\n", chname, fdin);
		return docID;
	}
	int bread;
	bread = read (fdin, &c, 1);
	if (docID <= 0)
		docID = 1;
	unsigned long crc = crc32(0L, Z_NULL, 0);
	while ( bread > 0 )
	{
		char textsizebuff [50];
		int textsizebufflen;
		char realfilename [2048];
		int realfilenamelen;
		char json [2048];
		int jsonlen = 0;

		realfilenamelen = 0;
		textsizebufflen = 0;

		long textsize;
		//get textsize
		textsizebuff[textsizebufflen++] = c;
		while ((c != ' ') && bread > 0)
		{
			bread = read (fdin, &c, 1);
			textsizebuff[textsizebufflen++] = c;
		}

		textsizebuff[textsizebufflen++] = '\0';

		textsize = atoi (textsizebuff);

		LOG_ZVM (ZLOGTIT, "text size", "ld", textsize, 1);


		if (textsize <= 0 || bread <=0)
			break;
		//get real filename
		bread = read (fdin, &c, 1);
		lastc = '\0';
		while ( !(c == '/' && lastc == '/') && bread > 0)
		{
			realfilename [realfilenamelen++] = c;
			lastc = c;
			bread = read (fdin, &c, 1);
		}
		realfilename[realfilenamelen-2] = '\0';
		LOG_ZVM (ZLOGTIT, "real file name", "s", realfilename, 1);
///////////////////
//get json
///////////////////
		bread = read (fdin, &c, 1);
		lastc = '\0';
		while ( !(c == '~' && lastc == '~') && bread > 0)
		{
			json [jsonlen++] = c;
			lastc = c;
			bread = read (fdin, &c, 1);
		}
		json[jsonlen-1] = '\0';
///////////////////
// end read json
///////////////////
		LOG_ZVM (ZLOGTIT, "json length", "zu", strlen (json), 2);
		LOG_ZVM (ZLOGTIT, "json", "s", json, 3);

		//read text data.
		char *buff;
		buff = (char *) malloc (textsize + 1);
		bread = read (fdin, buff, textsize);
		crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, (const Bytef*) realfilename, strlen (realfilename));
		printf ("realfilename=*%s*, crc=%lu\n", realfilename, crc);
		LOG_ZVM (ZLOGTIT, "crc23", "lu", crc, 1);



		printdochead (fd, crc);

		unsigned long bwrite = 0;
		//write_XML_Elemet ()
		bwrite += (unsigned long) write_XML_Elemet (fd, "meta", json);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get tags CONTENT_LENGTH && X_TIMESTAMP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		unsigned int a = 0;
		int linesymcount =0;
		char *startline = strstr (json, "{") + 1;
		char *pContentLength = NULL;
		char *pTimeStamp = NULL;
		for (a = 0; a < strlen (json); a++)
		{
			linesymcount++;
			if (json[a]  == '\n')
			{
				char *jsonline = (char *) malloc (sizeof (char) * linesymcount + 1);
				strncpy (jsonline, startline, linesymcount);
				if (jsonline[linesymcount - 1] == ',')
					jsonline[linesymcount - 1] = '\0';
				else
					jsonline[linesymcount] = '\0';
				LOG_ZVM (ZLOGTIT, "json line", "s", jsonline, 2);
				startline = json + a;

				if (strstr (jsonline, "\"") != NULL )
				{
//					printf ("%s, key = %s\n", jsonline, getKey (jsonline));
					int b = 0;
					for (b = 0; b < blank_attr_count; b++)
						if (MY_STRNCMP (blank_attr_list[b], getKey (jsonline)) == 0)
						{
							bwrite += (unsigned long) write_XML_Elemet (fd, getKey (jsonline), getValue (jsonline));
						}

					for (b = 0; b < blank_field_count; b++)
						if (MY_STRNCMP (blank_field_list[b], getKey (jsonline)) == 0)
						{
							bwrite += (unsigned long) write_XML_Elemet (fd, getKey (jsonline), getValue (jsonline));
						}
					for (b = 0; b < fl.fieldCount ; b++)
						if (MY_STRNCMP ( fl.fields[b] , getKey (jsonline)) == 0)
						{
							bwrite += (unsigned long) write_XML_Elemet (fd, getKey (jsonline), getValue (jsonline));
						}
				}

				//
/*
				if (strstr ( jsonline,"CONTENT_LENGTH") != NULL)
				{
					pContentLength = getValue (jsonline);
				}
				if (strstr ( jsonline,"TIMESTAMP") != NULL)
				{
					pTimeStamp = getValue (jsonline);
				}
*/
				linesymcount = 0;
			}
		}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end get tag CONTENT_LENGTH && X_TIMESTAMP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// generate all words from META for metatags field
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		char *metawords = NULL;
		metawords = generateMetaWords (json);

		char *metawords_all = (char *) malloc (sizeof (char) * (strlen (json) + strlen (metawords) + 10 ));
		sprintf (metawords_all, "%s \n %s \n", json, metawords);
		bwrite += (unsigned long) write_XML_Elemet (fd, "metatags", metawords_all);
		buff[textsize] = '\0';
		bwrite += (unsigned long) write_XML_Elemet (fd, "content", buff);



/*
		bwrite = write (fd, "<metatags><![CDATA[", 19); // +9
		bwrite = write (fd, json, strlen (json));
		bwrite = write (fd, metawords, strlen (metawords));
		bwrite = write (fd, "]]></metatags>\n", 15); // +3

		bwrite = write (fd, "<filecontentlength><![CDATA[", 28); //+9
		bwrite = write (fd, pContentLength, strlen (pContentLength));
		bwrite = write (fd, "]]></filecontentlength>\n", 24); //+3

		bwrite = write (fd, "<TIMESTAMP>", 11);
		bwrite = write (fd, pTimeStamp, strlen (pTimeStamp));
		bwrite = write (fd, "</TIMESTAMP>\n", 13);
																																																																																																																																																																																																																																																									//write json indexed field
		bwrite = write (fd, "<content><![CDATA[", 18);//+9-1(\n)
		bwrite = write (fd, buff, textsize);
		bwrite = write (fd, "]]></content>\n", 14);//+3
*/

		free(buff);
		free(metawords);
		free (pContentLength);
		printdocfooter (fd);
		docID++;

		LOG_ZVM (ZLOGTIT, "doc count", "d", docID, 1);

		bread = read (fdin, &c, 1);
	}
	return docID;
}


/*
 * get data (file doc, txt, docx, pdf ... etc.) from channel and save file data to temporary file
 * return struct - tempfilename, realfilename, size of file
 *
*/
struct filemap getfilefromchannel (char * chname, char 	*prefix)
{
	printf ("(getfilefromchannel) process file %s\n", chname);
	int fdin, fdout, i;
	struct filemap fmap;
	char nbytes[20]; //
	char *buff = NULL;
	char *resizebuff;
	long buffsize;
	char ext[strlen (chname)];
	long realfilesize;
	long bread, readcount, blockreadsize, totalreadsize;
	char tempfilename[strlen (chname) + strlen (prefix) + 1];
	fmap.realfilesize = 0;
	i = 0;
	getext (chname, ext);

	sprintf (tempfilename, "%s/temp.tmp", prefix);

	strcpy (fmap.tempfilename, tempfilename);

	if (!chname)
	{
		printf ("*** ZVM Error open in channel name\n");
		return fmap;
	}

	fdin = open (chname, O_RDONLY);
	if (fdin < 0)
	{
		printf ("*** ZVM Error open channel %s\n", chname);
		return fmap;
	}


////////////////////////////////////////////////////////////////////////
//	int remretcode = remove (tempfilename);
//	if (remretcode == 1)
//		printf ("error deleting previous temp file\n");
////////////////////////////////////////////////////////////////////////

	fdout = open (tempfilename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if (fdout < 0)
	{
		printf ("*** ZVM Error create temp file %s\n", tempfilename);
		return fmap;
	}

	blockreadsize = READWRITEBUFFSIZE;
	totalreadsize = 0;
	readcount = 0;
	buffsize = blockreadsize;
	totalreadsize = 0;
	bread = 0;
	buff = (char *) malloc (blockreadsize);
	while ((bread = (long)read (fdin, buff + readcount * blockreadsize, blockreadsize)) > 0)
	{
		buffsize += blockreadsize;
		resizebuff = (char *) realloc ((char *)buff, buffsize);

		if (resizebuff != NULL )
		{
			buff = resizebuff;
		}
		else
		{
			fmap.realfilesize = 0;
			return fmap;
		}
		totalreadsize += bread;
		readcount++;
	}
	printf ("totalreadsize = %ld\n", totalreadsize);
	if (totalreadsize <= 0)
	{
		close (fdin);
		close (fdout);
		free (buff);
		fmap.realfilesize = 0;
		return fmap;
	}

	if (totalreadsize <= 1024 * 1024)
	{
		printf ("test readed data if size <= 1024");
		fwrite (buff, sizeof (char), totalreadsize, stdout);
		printf ("**\n");
	}

	// FIX if an incorrect format of the readed data
	i = 0;
	while (isdigit (buff[i]))
	{
		nbytes [i] = buff[i];
		i++;
	}
	nbytes [i] = '\0';

	realfilesize = atoi (nbytes);
	printf ("realfilesize = %ld\n", realfilesize);
	i++; // rewind space
	int j;
	for (j = 0;i < totalreadsize - 1;i++, j++)
	{
		if (buff [i] == '/' && buff [i + 1] == '/')
			break;
		fmap.realfilename[j] = buff[i];
	}
	fmap.realfilename[j-1] = '\0';
	i += 2; //rewind space and //

	// json data size = total size - header size - file size
	size_t jsonsize = totalreadsize - realfilesize - i + 10; // 10 - stock :)
	char *json = (char *) malloc (jsonsize * sizeof (char));

	for (j = 0;i < totalreadsize - 1;i++, j++)
	{
		if (buff [i] == '~' && buff [i + 1] == '~')
			break;
		json[j] = buff[i];
	}
	json[j-1] = '\0';
	i += 2; //rewind space and //
	fmap.json = json;
	//
	int bwrite = 0;
	if (realfilesize > 0)
		bwrite = write (fdout, buff + i, realfilesize);
	fmap.realfilesize = bwrite;
	close (fdin);
	close (fdout);
	free (buff);
	return fmap;
}


/*
 * get data (file doc, txt, docx, pdf ... etc.) from channel and save file data to temporary file in zerovm FS
 * return struct - tempfilename, realfilename, size of file, json
 *
 * packet size 			10 bytes
 * real filename size 	10 bytes
 * file name 			N  bytes
 * json size			10 bytes
 * json 				N  bytes
 * filecontetn size		10 bytes
 * file contetnt		N bytes
*/
struct filemap extractorfromfilesender (char * chname, char *prefix)
{
	int fdIN, fdOUT;
	struct filemap fmap;
	char tempfilename[strlen (prefix) + 12]; // prefix + temp.tmp

	int iNumBuffSize = PACKET_NUMBER_BLOCK_SIZE;


	fmap.realfilesize = 0;
	fdIN = open (chname, O_RDONLY);
	if (fdIN < 0)
	{
		printf ("***ZVM Error open channel %s\n", chname);
		return fmap;
	}

	sprintf (tempfilename, "%s/temp.tmp", prefix);
	strcpy (fmap.tempfilename, tempfilename);

	fdOUT = open (tempfilename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if (fdOUT < 0)
	{
		printf ("*** ZVM Error create temp file %s\n", tempfilename);
		return fmap;
	}

////////////////////////////////////////////////////////////////////
// read data from channel
////////////////////////////////////////////////////////////////////

	char *buff = NULL;
	char *resizebuff = NULL;
	long buffsize;
	long bread, readcount, blockreadsize, totalreadsize;

	blockreadsize = READWRITEBUFFSIZE;
	totalreadsize = 0;
	readcount = 0;
	buffsize = blockreadsize;
	totalreadsize = 0;
	bread = 0;
	buff = (char *) malloc (blockreadsize);

	while ((bread = (long)read (fdIN, buff + readcount * blockreadsize, blockreadsize)) > 0)
	{
		buffsize += blockreadsize;
		resizebuff = (char *) realloc ((char *)buff, buffsize);

		if (resizebuff != NULL )
		{
			buff = resizebuff;
		}
		else
		{
			fmap.realfilesize = 0;
			return fmap;
		}
		totalreadsize += bread;
		readcount++;
	}

	LOG_ZVM (ZLOGTIT, "total read size", "ld", totalreadsize, 1);


	if (totalreadsize <= 0)
	{
		close (fdIN);
		close (fdOUT);
		free (buff);
		fmap.realfilesize = 0;
		return fmap;
	}
////////////////////////////////////////////////////////////////////
// END read data from channel
////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// parse readed data
////////////////////////////////////////////////////////////////////


	char numberbuff [iNumBuffSize + 1];
	long bytesparsed = 0;
	// read packet size 10 bytes
	strncpy (numberbuff, buff, iNumBuffSize);
	numberbuff [iNumBuffSize + 1]  = '\0';
	bytesparsed += iNumBuffSize;

	long checkpacketsize = 0;
	long filenamelength = 0;
	long jsonlength = 0;
	long filelength = 0;
	// check packet size
	checkpacketsize = atol (numberbuff);
	LOG_ZVM (ZLOGTIT, "detected packet size", "ld", checkpacketsize, 1);

	if (checkpacketsize != totalreadsize)
		printf ("*** Warning conflict packet size. Real readed bytes %lu, packet size specified in header %lu\n", totalreadsize, checkpacketsize);

	// read file name size
	strncpy (numberbuff, buff + bytesparsed, iNumBuffSize);
	numberbuff[iNumBuffSize + 1] = '\0';
	filenamelength =  atol (numberbuff);
	bytesparsed += iNumBuffSize;

	// read filename
	strncpy (fmap.realfilename, buff + bytesparsed, filenamelength);
	fmap.realfilename[filenamelength] = '\0';
	bytesparsed += filenamelength;
	LOG_ZVM (ZLOGTIT, "real file name size", "zu", strlen (fmap.realfilename), 2);
	LOG_ZVM (ZLOGTIT, "real filename", "s", fmap.realfilename, 1);

	// read json length
	strncpy (numberbuff, buff + bytesparsed, iNumBuffSize);
	numberbuff[iNumBuffSize + 1] = '\0';
	jsonlength =  atol (numberbuff);
	bytesparsed += iNumBuffSize;
	LOG_ZVM (ZLOGTIT, "json size", "ld", jsonlength, 2);

	//read json
	char *json = (char *) malloc (jsonlength * sizeof (char) + 1);
	strncpy (json, buff + bytesparsed, jsonlength);
	json [jsonlength] = '\0';
	fmap.json = json;
	bytesparsed += jsonlength;
	LOG_ZVM (ZLOGTIT, "json", "s", json, 3);
	LOG_ZVM (ZLOGTIT, "fmap.json size", "zu", strlen (fmap.json), 2);

	// read filelength
	strncpy (numberbuff, buff + bytesparsed, iNumBuffSize);
	numberbuff[iNumBuffSize + 1] = '\0';
	filelength =  atol (numberbuff);
	bytesparsed += iNumBuffSize;
	LOG_ZVM (ZLOGTIT, "filelength", "ld", filelength, 1);

	// write file content to temp file
	long bwrite = 0;
	bwrite = write (fdOUT, buff + bytesparsed, filelength);

	//check
	if ( bwrite != filelength)
		printf ("***Something wrong ... \n");

	fmap.realfilesize = bwrite;
	close (fdIN);
	close (fdOUT);
	free (buff);
	return fmap;
}

/*
 * read data from input file and put it in to output channel to extractor
 * in format
 *
 * packet size 			10 bytes
 * real filename size 	10 bytes
 * file name 			N  bytes
 * json size			10 bytes
 * json 				N  bytes
 * filecontetn size		10 bytes
 * file contetnt		N bytes
 */
void filesender2extractor (char * inputchname, char * outputchname, char *realfilename, char *json, int bMetaOnly)
{
	int fdIN, fdOUT;
	unsigned long uPacketSize = 0;
	unsigned long fsize = 0;
//	char bMetaOnly = 0;
	long bwrite, bread;

	fdIN = open (inputchname, O_RDONLY);
	fdOUT = open (outputchname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);

	if (fdIN < 0)
	{
		printf("*** Error open input channel %s, fdIN=%d\n", inputchname, fdIN);
	}
	if (fdOUT < 0)
	{
		printf("*** Error open output channel %s, fdOUT=%d\n", inputchname, fdOUT);
	}

	fsize = getfilesize_fd (fdIN, NULL, 0);

	if (strstr(outputchname,"other") == NULL)
		bMetaOnly = 0;
	else
	{
		bMetaOnly = 1;
	}

	uPacketSize = 10;							// 10 packet length
	uPacketSize += strlen (realfilename) + 10;	// real file name + length
	uPacketSize += strlen (json) + 10; 			// json length + length
	if (bMetaOnly == 1)
		uPacketSize += 15; 						// 10 size  + "other"
	else
		uPacketSize += fsize + 10; 				// 10 size  + file content

	char *buff = NULL;
	buff = (char *) malloc (sizeof (char) * uPacketSize);

	if (bMetaOnly == 1)
	{
		sprintf (buff, "%10lu%10zu%s%10zu%s%10zu%s",uPacketSize, strlen (realfilename), realfilename, strlen (json), json, 5, "other");
	}
	else
	{
		sprintf (buff, "%10lu%10zu%s%10zu%s%10lu",uPacketSize, strlen (realfilename), realfilename, strlen (json), json, fsize);
		bread = read (fdIN, buff + strlen (buff), fsize);
	}

	bwrite = write (fdOUT, buff, uPacketSize);

	close (fdIN);
	close (fdOUT);

	LOG_ZVM (ZLOGTIT, "bytes write to output channel", "ld", bwrite, 1);

/*
	if (buff != NULL)
		free (buff);
*/
	return;
}

int puttext2channel (char *bufftext, long size, char *realfilename, char *json, int channelfd)
{
	long totalbytewrite, bytewrite;
	bytewrite = 0;
	totalbytewrite = 0;
	char tempbuff [strlen (realfilename) > 20 ? strlen (realfilename) + 1 : 20];
	if (channelfd < 0)
	{
		printf ("*** ZVM error write text to file. Wrong file descriptor\n");
		return -1;
	}
	sprintf (tempbuff, "%ld ", size);
	bytewrite = write (channelfd, tempbuff, strlen (tempbuff));// write size of text
	if (bytewrite == 0)
	{
		printf("*** ZVM error write data to file\n");
		return -1;
	}
	sprintf (tempbuff, "%s //", realfilename);
	totalbytewrite += bytewrite;

	bytewrite = write (channelfd, tempbuff, strlen (tempbuff)); // write real filename + " //"
	if (bytewrite == 0)
		{
			printf("*** ZVM error write data to file\n");
			return -1;
		}
	totalbytewrite += bytewrite;

	bytewrite = write (channelfd, json, strlen (json)); // write json data
	if (bytewrite == 0)
		{
			printf("*** ZVM error write json data to file\n");
			return -1;
		}
	totalbytewrite += bytewrite;

	bytewrite = write (channelfd, "~~", 2); // write "//"
	if (bytewrite == 0)
		{
			printf("*** ZVM error write data to file\n");
			return -1;
		}
	totalbytewrite += bytewrite;


	bytewrite = write (channelfd, bufftext, size);  // write filtered text buffer
	if (bytewrite == 0)
		{
			printf("*** ZVM error write data to file\n");
			return -1;
		}
	totalbytewrite += bytewrite;

	return totalbytewrite;
}

/*
 * filtering text buffer
*/

int getfilteredbuffer (const char *buff, long bufflen, char *filteredbuff)
{
	long i;
	char lastc = ' ';
	long filteredbuffsize;
	filteredbuffsize =0;
	for (i = 0; i < bufflen; i++)
	{
		if (isalnum(buff[i]) || ((isspace (buff[i]) && !isspace (lastc))))
		{
			lastc = buff[i];
			filteredbuff[filteredbuffsize++] = buff[i];
			//printf("%c %d\n", buff[i], buff[i]);
		}
	}
	filteredbuff [filteredbuffsize] = '\0';
	return filteredbuffsize;
}

void printstat (long mainbytes, long deltabytes, int filecount, const char * mainindexname, char *deltaindexname)
{
	float konvmainbytes = (mainbytes / 1024) > 0 ? (mainbytes / (1024 * 1024)) > 0 ? (mainbytes / (1024 * 1024 * 1024)) > 0 ? (mainbytes / (1024.0 * 1024.0 * 1024.0)) : (mainbytes / (1024.0 * 1024.0)) : (mainbytes / 1024.0) : mainbytes;
	float konvdeltabytes = (deltabytes / 1024) > 0 ? (deltabytes / (1024 * 1024)) > 0 ? (deltabytes / (1024 * 1024 * 1024)) > 0 ? (deltabytes / (1024.0 * 1024.0 * 1024.0)) : (deltabytes / (1024.0 * 1024.0)) : (deltabytes / 1024.0) : deltabytes;
	const char *strmain = (mainbytes / 1024) > 0 ? (mainbytes / (1024 * 1024)) > 0 ? (mainbytes / (1024 * 1024 * 1024)) > 0 ? "Gb" : "Mb" : "Kb" : "B";
	const char *strdelta = (deltabytes / 1024) > 0 ? (deltabytes / (1024 * 1024)) > 0 ? (deltabytes / (1024 * 1024 * 1024)) > 0 ? "Gb" : "Mb" : "Kb" : "B";
	if ( getZVMLogLevel() == 1 )
	{
		printf ("***ZVMLog [LogLevel>0] [%s] \t %ld \tbytes, \t(%5.2f %s)\n", mainindexname, mainbytes, konvmainbytes, strmain);
		printf ("***ZVMLog [LogLevel>0] [%s] \t %ld \tbytes, \t(%5.2f %s)\n", deltaindexname, deltabytes, konvdeltabytes, strdelta);
		printf ("***ZVMLog [LogLevel>0] [file count] \t %d\n", filecount);
	}
	return;
}

void unpackindex_fd (char *	devname)
{
#ifdef TEST
	printf ("*** ZVM (unpackindexfd) start unpack from %s\n", devname);
#endif
	int fdinfile;
	char *dirName = (char*)INDEXDIRNAME;
  	DIR *dir;
	dir = opendir(dirName);

	if (dir == NULL)
	{
		int result=mymakedir(dirName);
		if (result != 0)
			printf ("*** ZVM Error can`t create directory %s\n", dirName);
	}
	else
		closedir (dir);
	int MAXREAD = 1024;
	char readbuf [MAXREAD];
	int letcount;
	letcount = 0;
	fdinfile = open (devname, O_RDONLY);
	if (fdinfile <= 0)
	{
		printf ("*** ZVM error input indexpack file\n");
		return;
	}
	char c;
	int bread;
	bread = 1;
	long deltabytes = 0;
	long mainbytes = 0;
	int filecount = 0;
	while (bread > 0)
	{
		// получение количества байт в сохраненном файле
		letcount = 0;
		//c = getc (infile);
		bread = read (fdinfile, &c, 1);

		if (bread == 0)
		{
#ifdef TEST
			printf ("*** ZVM unpack index completed successfully!\n");
#endif
			close (fdinfile);
#ifdef TEST
			printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char*) DELTAINDEX);
#endif
			return;
		}
		//sprintf ("c=%c\n", c);
		/*
		while (c != '{' && bread >= 0)
		{
			bread = read (fdinfile, &c, 1);
		}
		*/

		if (c != '{')
		{
#ifdef TEST
			int tellsize = lseek (fdinfile, 0, SEEK_CUR);
			int bytesleft = 0;
#endif
			printf ("Error *\n");
/*
			while ( (bread = read (fdinfile, &c, 1)) > 0)
			{
				//printf ("%c", c);
				bytesleft++;
			}
*/
#ifdef TEST
			printf("*** ZVM Error format packfile c=*%c*, current position = %d, bytes left = %d\n", c, tellsize, bytesleft);
#endif
			close (fdinfile);
#ifdef TEST
			printstat (mainbytes, deltabytes, filecount, (char*) MAININDEX, (char *) DELTAINDEX);
#endif
			return;
		}
		bread = read (fdinfile, &c, 1);
		while (!isspace(c) && bread >=0) {
			readbuf[letcount++] = c;
			bread = read (fdinfile, &c, 1);
		}
		readbuf [letcount] = '\0';
		//printf ("readbuf=%s\n", readbuf);
		long filelen = 0;
		//filelen = 0;
		filelen = atoi (readbuf);
		//printf ("filelen detected = %ld\n", filelen);
		//получение имени файла
		letcount =0;
		bread = read (fdinfile, &c, 1);
		while (c != '\n')
		{
			readbuf [letcount++] = c;
			bread = read (fdinfile, &c, 1);
		}
		readbuf [--letcount] = '\0';
		//printf ("readbuf=%s\n", readbuf);
		int fdefile;
		fdefile = open (readbuf, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
		if (fdefile <= 0)
		{
			printf ("*** ZVM Error open %s file for write\n", readbuf);
			close (fdinfile);

			return;

		}
		char *buff = NULL;
		int blocksize = READWRITEBUFFSIZE;
		int readb = 0;//read(fdinfile, buff,filelen);
		int writeb = 0;//write(fdefile, buff,readb);

		long countreadbytes = 0;
		long countwritebytes = 0;

		buff = (char *) malloc (blocksize * sizeof (char));
		if (filelen > 0)
		{

			//printf ("filelen = %d, block size = %d \n", filelen, blocksize);
			if (blocksize > filelen)
				blocksize = filelen;
			int i=filelen / blocksize + 1;
			//printf ("readcount %d\n", i);
			for (; i > 0; i--)
			{

				if (filelen < blocksize)
					blocksize = filelen;
				if (i == 1)
					blocksize = filelen - countreadbytes;
				//printf ("blocksize = %d\n", blocksize);
				//printf ("filelen = %d, blocksize = %d \n", filelen, blocksize);
				readb = read(fdinfile, buff, blocksize);
				if (readb > 0)
					writeb = write(fdefile, buff, readb);
				else
					writeb = readb;
				countreadbytes += readb;
				countwritebytes += writeb;
				//printf ("countreadbytes=%d, current readbytes=%d \n", countreadbytes, readb);
			}
			readb = countreadbytes;
			writeb = countwritebytes;
		}
		// if (filelen > 0)
/*
		if (filelen > 0)
		{
			buff = (char *) malloc (filelen);
			readb = read(fdinfile, buff,filelen);
			writeb = write(fdefile, buff,readb);
		} // if (filelen > 0)
*/

		char *indexnameptr = NULL;
		if ((indexnameptr = strstr (readbuf,DELTAINDEX)) != NULL )
			deltabytes += filelen;

		indexnameptr = NULL;
		if ((indexnameptr = strstr (readbuf,MAININDEX)) != NULL )
			mainbytes += filelen;
		filecount++;

		free (buff);
		close (fdefile);
		if (readb != writeb)
			printf ("*** Warning while unpacking index file, readb = %d, writeb = %d\n", readb, writeb);
#ifdef TEST
		else
			printf ("*** ZVM unpack file %s (%d bytes)  - OK!\n", readbuf, writeb);
#endif
		bread = read (fdinfile, &c, 1);
		//printf ("c = %c\n", c);
		int loopcount =0;
		while (c != '\n')
		{
			loopcount++;
			bread = read (fdinfile, &c, 1);
			//printf ("*%c", c);
		}
		//printf ("loopcount = %d\n", loopcount);
	}
#ifdef TEST
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char* ) DELTAINDEX);
#endif
	close (fdinfile);
}


/*
 *
 * unpacking files data from pack file
 *
 * */
void newbufferedunpack (char *	devname)
{

	LOG_ZVM (ZLOGTIT, "index pack device (input)", "s", devname, 2);
	int fdinfile;
	char *dirName = (char*)INDEXDIRNAME;
	LOG_ZVM (ZLOGTIT, "index directory name", "s", dirName, 2);

  	DIR *dir;
	dir = opendir(dirName);
	if (dir == NULL)
	{
		int result=mymakedir(dirName);
		if (result != 0)
			printf ("*** ZVM Error can`t create directory %s\n", dirName);
	}
	else
		closedir (dir);
	int MAXREAD = 1024;
	char readbuf [MAXREAD];
	int letcount;
	letcount = 0;
	fdinfile = open (devname, O_RDONLY);
	if (fdinfile <= 0)
	{
		printf ("*** ZVM error input indexpack file\n");
		return;
	}
	int bread = 1;
	long deltabytes = 0;
	long mainbytes = 0;
	int filecount = 0;

	char blocksizestr [10];
	size_t blocksize;
	LOG_ZVM (ZLOGTIT, "read and write buff size", "zu", READWRITEBUFFSIZE, 2);
	while (bread > 0)
	{
		if (filecount > 50)
		{
			printf ("Error unpack index files\n");
			break;
		}
		// получение количества байт в имени сохраненного файла
		bread = read(fdinfile, blocksizestr, 10);
		if (bread <= 0)
		{
			break;
		}
		blocksizestr[10] = '\0';
		blocksize = atoi (blocksizestr);

		// read file name
		bread = read (fdinfile, readbuf, blocksize);
		readbuf [blocksize] = '\0';
		LOG_ZVM (ZLOGTIT, "file name", "s", readbuf, 2);

		// получение количества байт в сохраненном файле
		bread = read(fdinfile, blocksizestr, 10);
		blocksizestr[10] = '\0';
		blocksize = atoi (blocksizestr);
		LOG_ZVM (ZLOGTIT, "file size", "zu", blocksize, 2);

		size_t filelen = 0;
		filelen = blocksize;

		int fdefile;
		fdefile = open (readbuf, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
		if (fdefile <= 0)
		{
			printf ("*** ZVM Error open %s file for write. Increasing the current offset of the file descriptor on %zu bytes  and skip save this file.\n", readbuf, filelen);
			// add check type of device. If we can use lseek
			bread = lseek (fdinfile, filelen, SEEK_CUR);
			continue;
		}

		// readind and save data file
		char *buff = NULL;
		size_t blockreadsize = READWRITEBUFFSIZE;

		int readb = 0;//read(fdinfile, buff,filelen);
		int writeb = 0;//write(fdefile, buff,readb);

		long countreadbytes = 0;
		long countwritebytes = 0;

		buff = (char *) malloc (blockreadsize * sizeof (char));
		if (filelen > 0)
		{

			if (blockreadsize > filelen)
				blockreadsize = filelen;
			int i=filelen / blockreadsize + 1;
			for (; i > 0; i--)
			{
				if (filelen < blockreadsize)
					blockreadsize = filelen;
				if (i == 1)
					blockreadsize = filelen - countreadbytes;
				readb = read(fdinfile, buff, blockreadsize);
				if (readb > 0)
					writeb = write(fdefile, buff, readb);
				else
					writeb = readb;
				countreadbytes += readb;
				countwritebytes += writeb;
			}
			readb = countreadbytes;
			writeb = countwritebytes;
		}

		// statistic
		char *indexnameptr = NULL;
		if ((indexnameptr = strstr (readbuf,DELTAINDEX)) != NULL )
			deltabytes += filelen;

		indexnameptr = NULL;
		if ((indexnameptr = strstr (readbuf,MAININDEX)) != NULL )
			mainbytes += filelen;
		filecount++;

		free (buff);
		close (fdefile);
		if (readb != writeb)
			printf ("*** Warning while unpacking index file, readb = %d, writeb = %d\n", readb, writeb);
//			LOG_ZVM (ZLOGTIT, "unpacked file size", "d", writeb, 2);
	}
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char* ) DELTAINDEX);
	close (fdinfile);
}

/*
 ZVM Function for packing all index files from directory in ZeroVM FS, specified in Zsphinx.conf to /dev/output device
*/

void newbufferedpack (char *devname, char *dirname)
{

	LOG_ZVM (ZLOGTIT, "start packing", "s", "OK", 2);
	LOG_ZVM (ZLOGTIT, "device to pack (output)", "s", devname, 2);
	LOG_ZVM (ZLOGTIT, "path to files", "s", dirname, 2);

	int fdpackfile;

	fdpackfile = open (devname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if ( fdpackfile  <= 0 )
	{
		printf ("*** ZVM Error open packfile (write)%s\n", devname);
		return;
	}

	char *indexpath;//deirectory with  index files and zspfinx.conf
	indexpath = dirname;
  	DIR *dir;
	struct dirent *entry;
	dir = opendir(indexpath);
	char *newpath;

	if (!dir)
		printf ("*** ZVM Error open DIR %s\n", indexpath);
	int blocksize = READWRITEBUFFSIZE; // 10 Mb

	LOG_ZVM (ZLOGTIT, "read and write block size", "d", blocksize, 2);
	char *buff = NULL;
	buff = (char *) malloc (blocksize);

	long deltabytes = 0;
	long mainbytes = 0;
	int filecount = 0;

	while((entry = readdir(dir)))
	{
		if(entry->d_type != DT_DIR)
		{
			size_t size;
			size_t bread = 0;
			size_t bwrite;
			size_t bytecount;
			bytecount = 0;

			newpath = (char *) malloc (strlen (entry->d_name) + strlen(indexpath) + 2);
			sprintf(newpath, "%s/%s", indexpath, entry->d_name);
			int fd;

			fd = open (newpath, O_RDONLY);
			size = getfilesize_fd(fd, NULL, 0);

			char tempstr [strlen (newpath) + 12];
			// write header (10 bytes size of filename + filename + 10 bytes size of filedata)
			sprintf(tempstr, "%10zu%s%10zu", strlen (newpath), newpath, size);
			bwrite = write (fdpackfile, tempstr, strlen (tempstr));
			// write header (10 bytes size of filename + filename)


			//read and write file data
			if (size > 0)
			{
				while ((bread = read(fd, buff, blocksize)) > 0)
				{
					bytecount += bread;
					bwrite = write(fdpackfile, buff, bread);
				}
			} else
				bytecount = 0;

			close (fd);
			LOG_ZVM (ZLOGTIT, "packed file name", "s", newpath, 2);
			LOG_ZVM (ZLOGTIT, "packed file size", "d", size, 2);

			// for statistic data
			char *indexnameptr = NULL;
			if ((indexnameptr = strstr (newpath,DELTAINDEX)) != NULL )
				deltabytes += size;

			indexnameptr = NULL;
			if ((indexnameptr = strstr (newpath,MAININDEX)) != NULL )
				mainbytes += size;
			filecount++;
			//statistic
		}
	}
	free (buff);
	close (fdpackfile);
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char *) DELTAINDEX);
	LOG_ZVM (ZLOGTIT, "packed all files", "s", "OK", 2);
}

void bufferedpackindexfd (char * devname)
{
#ifdef TEST
	printf("*** ZVM (bufferedpackindexfd_) start pack index to %s \n", devname);
#endif
	int fdpackfile;

	fdpackfile = open (devname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);


	if ( fdpackfile  <= 0 )
	{
		printf ("*** ZVM Error open packfile (write)%s\n", devname);
		return;
	}

	char *indexpath = (char *)INDEXDIRNAME; // deirectory with  index files and zspfinx.conf
  	DIR *dir;
	struct dirent *entry;
	dir = opendir(indexpath);
	char *newpath;

	if (!dir)
		printf ("*** ZVM Error open DIR %s\n", indexpath);
	int blocksize = READWRITEBUFFSIZE; // 10 Mb
#ifdef TEST
	printf ("blocksize = %d\n", blocksize);
#endif
	char *buff = NULL;
	buff = (char *) malloc (blocksize);

	long deltabytes = 0;
	long mainbytes = 0;
	int filecount = 0;

	while((entry = readdir(dir)))
	{
		if(entry->d_type != DT_DIR)
		{
			size_t size;
			size_t bread = 0;
			size_t bwrite;
			size_t bytecount;
			bytecount = 0;

			newpath = (char *) malloc (strlen (entry->d_name) + strlen(indexpath) + 2);
			sprintf(newpath, "%s/%s", indexpath, entry->d_name);
			int fd;

			fd = open (newpath, O_RDONLY);
			size = getfilesize_fd(fd, NULL, 0);

			char tempstr [strlen (newpath) * 2 + 50];

			sprintf(tempstr, "{%zu %s}\n", size, newpath);
			bwrite = write (fdpackfile, tempstr, strlen (tempstr));
			if (size > 0)
			{
				while ((bread = read(fd, buff, blocksize)) > 0)
				{
					//bread = read (fd, buff, blocksize);
					bytecount += bread;
					bwrite = write(fdpackfile, buff, bread);
				}
			} else
				bytecount = 0;
			sprintf (tempstr, "{%s}\n", newpath);
			bwrite = write (fdpackfile, tempstr, strlen (tempstr));
			close (fd);
#ifdef TEST
			printf ("file %s (%d bytes) packed - OK!\n", newpath, (int) size);
#endif

			char *indexnameptr = NULL;
			if ((indexnameptr = strstr (newpath,DELTAINDEX)) != NULL )
				deltabytes += size;

			indexnameptr = NULL;
			if ((indexnameptr = strstr (newpath,MAININDEX)) != NULL )
				mainbytes += size;
			filecount++;
		}
	}
	free (buff);
	close (fdpackfile);
#ifdef TEST
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char *) DELTAINDEX);
	printf ("*** ZVM pack completed successfully!\n");
#endif
}

int mymakedir (char * newdir)
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
        if (buffer==NULL)
        {
                printf("Error allocating memory\n");
                return 1;
        }
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mkdir (buffer,0777) == 0)
    {
      free(buffer);
      return 0;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mkdir (buffer,0777) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 0;
}

void mylistdir (char *path)
{
  	DIR *dir;
	struct dirent *entry;
	char newpath[1024];
	dir = opendir(path);
	int len;
	if(dir == 0)
	{
		return;
	}
	while((entry = readdir(dir)))
	{

		printf ("%s/%s D_TYPE = %d\n",path, entry->d_name, entry->d_type);
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
	}
	closedir(dir);
}

/*
 *
 * */

int isNewWord (char * text, size_t pos)
{
	if ( isspace(text[pos]) && !isspace(text[pos + 1]))
		return 1;
	else
		return 0;
}

/*
 *
 * Receive seeking text, Start and End positions of searched words seeking text
 * Return pointer to text snippet with searched words
 *
 * */
char * getTextByHits (char *text, unsigned int uiStart, unsigned int uiEnd)
{
	char *s = (char *) malloc (sizeof (char ) * TEXT_SNIPPET_SIZE);
	size_t i;
	size_t tTextLength = 0;
	size_t tCurrentWordPos = 0;
	size_t tPosCount = 0;
	size_t tLastWordCharPos = 0, tStartCharPos = 0;
	unsigned int NotFoundPos = 16777000; 	// if the sphinx cannot find position of words in the text it return this magic number
	unsigned int LastPosInFile = 8388000; 	// if the searched word is the last in the document the sphinx returns the number largest than this magic
	size_t tSnippetSize = 0;
	int bFind = 0;
	if (text != NULL)
		tTextLength = strlen (text);
	else
		return NULL;

	LOG_ZVM (ZLOGTIT, "start pos", "u", uiStart, 1);

	if(uiStart >= NotFoundPos)
	{
		return NULL;
	}
	if (uiStart > LastPosInFile )
	{
		if (tTextLength >= TEXT_SNIPPET_SIZE)
		{
			strncpy (s, text + (tTextLength - TEXT_SNIPPET_SIZE), TEXT_SNIPPET_SIZE);
		}
		else
		{
			strncpy (s, text, tTextLength + 1);
		}
		return s;
	}

	if (uiStart > 1)
		uiStart -= 2;

	for ( i = 0; i < tTextLength - 1; i++)
	{
		if (isNewWord (text, i) == 1)
		{
			tLastWordCharPos = tCurrentWordPos;
			tCurrentWordPos = i;
			tPosCount++;
		}
		if (tPosCount == uiStart)
		{
			tStartCharPos = tLastWordCharPos;
			bFind = 1;
			break;
		}
	}

	if (bFind == 0)
		return NULL;

	if ((tStartCharPos + TEXT_SNIPPET_SIZE) < tTextLength)
		tSnippetSize = TEXT_SNIPPET_SIZE;
	else
	{
		tSnippetSize = tTextLength - tStartCharPos;
		if (tSnippetSize < TEXT_SNIPPET_SIZE  && tTextLength > TEXT_SNIPPET_SIZE)
		{
			tStartCharPos = tStartCharPos - (TEXT_SNIPPET_SIZE - tSnippetSize);
			tSnippetSize = TEXT_SNIPPET_SIZE;
		}
	}

	strncpy (s, text + tStartCharPos, tSnippetSize);
	s[tSnippetSize] = '\0';

	return s;
}

char * getTextByWords (char *text, char *words)
{
	if (text == NULL)
		return NULL;
	char *s = (char *) malloc (sizeof (char ) * strlen (text));

	strncpy (s, text, strlen(text) + 1);
	s[strlen(text)] = '\0';

	return s;
}

int getbufffromtxt (char *filename, char *buffer)
{
	long txtbuffsize;
	int fd = open (filename, O_RDONLY);
	if (fd < 0)
	{
		printf ("*** ZVM Error open %s file\n", filename);
		return -1;
	}
	txtbuffsize = getfilesize_fd(fd, NULL, 0);
	if (txtbuffsize < 0)
	{
		close (fd);
		return -1;
	}
	int bread = read (fd, buffer, txtbuffsize);
	if (bread < 0 || bread != txtbuffsize)
		printf ("***ZVM Error read data fron txt file %s\n", filename);
	close (fd);
	return txtbuffsize;
}

struct fileTypeInfo checkMAxFileSize (char *filename, size_t filesize)
{
	char *ext = (char *) malloc (sizeof (char) * strlen (filename));
	char *channelname= (char *) malloc (sizeof (char) * 50);
	getext (filename, ext);
	struct fileTypeInfo ft;
	ft.tFileSize = filesize;
	ft.sExt = ext;
	ft.bSaveFile = 1;
	ft.bPlainText = 0;

	LOG_ZVM (ZLOGTIT, "extension", "s", ext, 2);

//	if (getenv("CONTENT_TYPE") != NULL)
//		if (strstr (getenv("CONTENT_TYPE"), "text/plain" ) != NULL)
//		{
//			ft.bPlainText = 1;
//		}

	if ((strncmp (ext, "txt", 3) == 0 || strncmp (ext, "odt", 3) == 0 || strncmp (ext, "docx", 4) == 0) && filesize <= FS_MAX_TEXT_FILE_LENGTH)
	{
		sprintf (channelname, "/dev/out/txt");
	}
	else if ((strncmp (ext, "pdf", 3) == 0 || strncmp (ext, "doc", 3) == 0) && filesize <= FS_MAX_FILE_LENGTH)
	{
		sprintf (channelname, "/dev/out/%s", ext);
	}
	else
	{
		//if ( ft.bPlainText == 1 && filesize <= FS_MAX_TEXT_FILE_LENGTH)
		//	sprintf (channelname, "/dev/out/txt");
		//else
		//{
		sprintf (channelname, "/dev/out/other");
		//			ft.bSaveFile = 0;
		//}
	}
	ft.sChannelname = channelname;
	return ft;
}


size_t SaveFileFromInput (char *sSaveName, char **environ)
{
	size_t tFileLength = 0;
	size_t bread = 0, bwrite = 0;
	char buff [EX_READ_WRITE_SIZE];
	int fdIN;
	int fdOUT;
	char *fileName;
	struct fileTypeInfo fti;
	int bPlainText = 0;
	const char *Cont_Len = "CONTENT_LENGTH";
	const char *Cont_Typ = "CONTENT_TYPE";
	const char *Path_info = "PATH_INFO";

	if (getenv (Cont_Len) != NULL)
		tFileLength = atoll (getenv (Cont_Len));
	if (getenv (Path_info) != NULL)
	{
		fileName = (char *) malloc (sizeof (char) * strlen (getenv (Path_info)) + 1);
		sprintf  (fileName, getenv (Path_info));
	}

	if (getenv(Cont_Typ) != NULL )
		if (strstr (getenv(Cont_Typ), "text/plain" ) != NULL)
		{
			bPlainText = 1;
		}

	fti = checkMAxFileSize (fileName, tFileLength);

	if ( fti.bSaveFile == 1 )
	{
		fdIN = open (EX_SEARCH_MODE_INPUT, O_RDONLY);
		if (fdIN < 0)
		{
			printf ("*** ZVM Error open input channel %s\n", EX_SEARCH_MODE_INPUT);
			return 0;
		}

		fdOUT = open (sSaveName, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
		if (fdOUT < 0)
		{
			printf ("*** ZVM Error open input channel %s\n", EX_SEARCH_MODE_INPUT);
			return 0;
		}

		while ((bread = read (fdIN, buff, EX_READ_WRITE_SIZE)) > 0)
		{
			bwrite = write (fdOUT, buff, bread);
			tFileLength += bwrite;
		}

		close (fdIN);
		close (fdOUT);
	}

	return tFileLength;
}

struct p_options initOptions ()
{
	struct p_options popt;

	popt.bToOutput = 0;
	popt.bTextSearchMode = 0;
	popt.tStart = 0;
	popt.tEnd = 0;
	popt.sWords = NULL;
	popt.iMaxWordsLen = 1024;
	popt.sWords = (char *) malloc (sizeof (char) * popt.iMaxWordsLen);
	return popt;
}

struct p_options getOptions (int argc, char *argv[])
{
	struct p_options popt = initOptions ();;
	int i =0;

    for (i = 0; i < argc; i++)
    {
		LOG_ZVM (ZLOGTIT, "argv", "s", argv[i], 1);
    	if (argv [i][0] == '-')
    	{
    		if (i==0)
    			;
    		else if (strcmp (argv[i],"--save") == 0 || strcmp (argv[i],"--savexml") == 0)
			{
				popt.bToOutput = 1;
			}
    		else if ((i + 2) >= argc)
    			break;
   			else if (strcmp (argv[i],"--search") == 0 )
			{
   				LOG_ZVM (ZLOGTIT, "search mode", "s", "ON", 1);
   				popt.bTextSearchMode = 1;
   				popt.tStart = atoll (argv[i+1]);
   				popt.tEnd = atoll (argv[i+2]);
				i += 2;
			}
    	}
    	else if  ( strlen(popt.sWords) + strlen(argv[i]) + 1 < (size_t) popt.iMaxWordsLen )
    	{
    		strcat (popt.sWords, argv[i]);
    		strcat (popt.sWords, " ");
    	}
    }
	return popt;
}

char *myrealloc (char * buff, size_t *buffsize)
{
	char *newbuff;
/*	char *oldbuff;*/
	size_t newbuffsize = *buffsize * 2;
	newbuff = (char *) malloc (sizeof (char) * newbuffsize);
	if (newbuff)
	{
		memcpy (newbuff, buff, *buffsize);
		free (buff);
		buff = newbuff;
		*buffsize = newbuffsize;
	}
	return buff;
}

char *my_strtolower (char *str)
{
	if (str == NULL)
		return NULL;
	int i = 0;
	char *resstr = (char *) malloc (sizeof (char) * strlen (str) + 1);

	for (i = 0; i < strlen (str); i++)
	{
		resstr[i] = tolower (str[i]);
	}
	resstr[i] = '\0';
	return resstr;
}

char *generateJson (char **environ)
{
	size_t jsonmaxsize = 1024; //1024 - initial lengt of JSON data
	size_t jsonsize = 0;
	int keycount = 0;

	char *json = (char *) malloc (sizeof (char) * jsonmaxsize);

	const char *tagfilters [] = {
			"CONTENT_LENGTH",
			"CONTENT_TYPE",
			"HTTP_X_OBJECT_META",
			"HTTP_X_TIMESTAMP",
			"PATH_INFO"
	};

	const char *tagfilters_remove_prefix [] = {
			"HTTP_X_OBJECT_META_",
			"HTTP_X_"
	};


	int tagfilterscount = sizeof (tagfilters) / sizeof (char *);
	int tagfilters_remove_prefix_count = sizeof (tagfilters_remove_prefix) / sizeof (char *);

	sprintf (json, "{\n");
	jsonsize = strlen (json) - 1;

	int i;
	for ( i = 0; environ[i] != NULL; i++)
	{
		int envsize = strlen (environ[i]);
		char *pKey = (char *) malloc (envsize * sizeof (char));
		char *pVal = (char *) malloc (envsize * sizeof (char));

		char *pEq;
		int eqPos = 0;//

		if ((pEq = strstr(environ[i], "=")) != NULL)
			eqPos = pEq - environ[i];

		if (eqPos == 0)
		{
			free (pKey);
			free (pVal);
			continue;
		}

		//copy key && value
		strncpy ( pKey, environ[i], eqPos );
		strncpy ( pVal, environ[i] + eqPos + 1, envsize - eqPos );

		pKey [eqPos] = '\0';

		char *pFilterOK;
		int iFind = 0;
		int iFindTrim = 0;
		int j = 0;

		for ( j = 0; j < tagfilterscount; j++ )
		{
			if ( (pFilterOK = strstr (pKey, tagfilters[j])) != NULL )
			{
				iFind = 1;
			}
		}

		if (iFind == 0)
		{
			continue;
		}

		for ( j = 0; j < tagfilters_remove_prefix_count; j++)
		{
			if ( (pFilterOK = strstr (pKey, tagfilters_remove_prefix[j])) != NULL )
			{
				char * pKeyTrim = (char *) malloc ( sizeof (char) * strlen (pKey) );

				strncpy ( pKeyTrim, pKey + strlen (tagfilters_remove_prefix[j]) , strlen (pKey) - strlen (tagfilters_remove_prefix[j]) + 1);

				size_t addsize = strlen (pKeyTrim) +  strlen (pVal) + 8; //8 symbols plus -------  1 '\t' + 1 '=' + 4 '"' + 1 ',' + 1 '\n'

				if ((jsonsize + addsize) > jsonmaxsize )
					json = myrealloc (json, &jsonmaxsize);

				sprintf (json + jsonsize + 1, "\t\"%s\":\"%s\",\n",pKeyTrim, pVal);

				char pKeyVal [strlen(pKey) + strlen (pVal) + 2];
				sprintf (pKeyVal, "%s:%s", pKey, pVal);
				LOG_ZVM (ZLOGTIT, "searched key", "s", tagfilters_remove_prefix[j], 3);
				LOG_ZVM (ZLOGTIT, "Key:Val", "s", pKeyVal, 3);

				jsonsize += addsize;
				free (pKeyTrim);
				keycount++;
				iFindTrim = 1;
				break;
			}
		}
		if (iFindTrim == 0)
		{
			size_t addsize = strlen (pKey) +  strlen (pVal) + 8; //8 symbols plus -------  1 '\t' + 1 '=' + 4 '"' + 1 ',' + 1 '\n'
			if ((jsonsize + addsize) > jsonmaxsize )
				json = myrealloc (json, &jsonmaxsize);
			sprintf (json + jsonsize + 1, "\t\"%s\":\"%s\",\n",pKey, pVal);
			jsonsize += addsize;
		}
		free (pKey);
		free (pVal);
	}

	if (jsonsize + 2 > jsonmaxsize)
		json = myrealloc (json, &jsonmaxsize);
	sprintf (json + jsonsize - 1, "\n}");
	jsonsize += 2;

	LOG_ZVM (ZLOGTIT, "json key count", "d", keycount, 1);

	return json;
}

char *FilteringSnippet (char * s)
{
	char *snippet = NULL;
	char *wordbuff = NULL;
	int bFindText = 0;

	int snippetlen = 0, i = 0;

	if (s != NULL)
	{
		snippetlen = strlen (s);
	}
	else
		return NULL;

	snippet = (char *) malloc (sizeof (char) * snippetlen);

	wordbuff = (char *) malloc (sizeof (char) * snippetlen);

	int wcount = 0, totallen = 0;
	for (i  = 0; i < snippetlen; i++)
	{
		if (!isspace (s[i]))
			wordbuff [wcount++] = s[i];
		else
		{
			if (wcount == 0)
				continue;
			wordbuff[wcount] = '\0';
			sprintf (snippet + totallen, "%s ", wordbuff);
			totallen = strlen (snippet);
			wcount = 0;
			bFindText = 1;
		}
		if ((bFindText == 0) && (i == (snippetlen-1)))
		{
			wordbuff[wcount] = '\0';
			sprintf (snippet + totallen, "%s ", wordbuff);
			totallen = strlen (snippet);
		}
	}
	int flen = strlen (wordbuff);
	for (i = 0; i < flen; i++)
	{

	}
	return snippet;
}

char *toLower (char* s)
{
	int slen, i = 0;

	if (s == NULL)
	{
		return NULL;
	}
	else
		slen = strlen (s);
	for (i = 0; i < slen; i++)
		s[i] = tolower (s[i]);
	return s;
}

void PrintSnippet (char *text, char *realfilename, unsigned int uiStart, unsigned int uiEnd)
{
	printf ("\nfilename <%s>\n", realfilename);
	char *pSnippet = NULL;
	if (text != NULL)
		pSnippet = getTextByHits (text, uiStart, uiEnd);

	if (pSnippet != NULL)
		printf ("snippet <%s>\n", FilteringSnippet(pSnippet));
	else
	{
		char *json = generateJson(environ);
		char *filteredbuff = generateMetaWords(json);
		filteredbuff = FilteringSnippet(getTextByWords(filteredbuff, NULL));

		printf ("snippet <%s>\n", toLower(filteredbuff));
		printf ("metadata\n");
	}
	return;
}



