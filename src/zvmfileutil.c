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
	//char ext[len];
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

/*
 * return size of file in bytes by file descriptor if sizebyfilename=0
 *
 * and
 *
 * return size of file in bytes by file name if sizebyfilename=1
*/
long getfilesize_fd (int fd, char *filename, int sizebyfilename)
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


/*
* write to indexer file document header for sphinx xml format
*/
void printdochead (int fd, char *realfilename, int docID)
{
	char str[strlen (realfilename) + 50]; // 50 - stock
	int bwrite;

	sprintf (str, "<sphinx:document id=\"%d\">\n", docID);
	bwrite = write (fd, str, strlen (str));

	sprintf (str, "<filename>%s</filename>\n", realfilename);
	bwrite = write (fd, str, strlen (str));
	return;
}

void printjson (int fd, char *json)
{
	// <meta> </meta>;
	char str [strlen (json) + 50]; // 50 - stock
	int bwrite = 0;

	sprintf (str, "<meta>%s</meta>\n", json);
	bwrite = write (fd, str, strlen (str));

	return;
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
 * get text data from network in format size of text, real filename, text data
*/
int getdatafromchannel (int fd, char *chname, int docID)
{
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
	printf ("%s\n", chname);
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
		printf ("textsizebufflen = %d\n", textsizebufflen);
		textsize = atoi (textsizebuff);

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

		//read text data.
		printf ("read data\n");
		char *buff;

		buff = (char *) malloc (textsize + 1);
		bread = read (fdin, buff, textsize);
//		printf ("bread = %d\n", bread);

		/*
		 * write to xml pipe readed data
		*/

		//

		crc = crc32(crc, (const Bytef*) realfilename, strlen (realfilename));

		printdochead (fd, realfilename, crc);

		//printf ("%s\n", json);
		printjson (fd, json);

		char *metawords = (char *) malloc (sizeof (char) * strlen (json));

		int a = 0;
		// filtering json
		for (a = 0; a < strlen (json); a++)
		{
			if (json [a] != '"' && json [a] != ':' && json [a] != ',' && json [a] != '{' && json [a] != '}' )
			{
				metawords[a] = json [a];
				//printf ("json = %c metawords %c\n", json [a], metawords [a]);
			}
			else
			{
				//printf ("**** %c \n", json [a]);
				metawords[a] = ' ';
			}
			if (json [a] == '}')
				metawords[a] = '\0';

			if ((json [a] == '"') || (json [a] == '{') || (json [a] == '}') || (json [a] == ':') || (json [a] == ',') || (json [a] == '/') || (json [a] == '_'))
				json [a] = ' ';
		}


		//printf ("%s\n\n\n %s\n",json,metawords);

/*
		size_t jsonstrlen = 1024;
		char *jsonstr = (char *) malloc (sizeof (char) * jsonstrlen);
		char *mvastr = (char *) malloc (sizeof (char) * strlen (json));
		size_t mvapos = 0;
		int a = 0, b =0;
		for (a = 0; a < strlen (json); a++)
		{
			if ( (json [a] == ',') || (json [a] == '}') )
			{
				jsonstr [b] = '\0';
				// разбираем строку. отбираем key && value
				int k =0, l =0;
				int last;
				char tempstr1 [b]; // stores key or value
				char tempstr2 [b]; // stores single words in key or value
				for (k = 0; k <= b; k++)
				{
					if (jsonstr [k] != '"')
						tempstr1 [l++] = jsonstr [k];
					if ((jsonstr [k] == ':') || (jsonstr [k] == ',') || (jsonstr [k] == '\0'))
					{
						crc = crc32(crc, (const Bytef*) tempstr1, l);
						printf ("%lu \n", crc);
						sprintf (mvastr + mvapos, " %lu,\n", crc);
						mvapos = strlen (mvastr);
						l = 0;
					}
				}
				b = 0;
				continue;
			}
			jsonstr [b++] = json [a];
		}
*/
		//
		int bwrite;
		//write json indexed field
		bwrite = write (fd, "<metatags>", 10);
		bwrite = write (fd, json, strlen (json));
		bwrite = write (fd, metawords, strlen (metawords));
		bwrite = write (fd, "</metatags>", 11);
																																																																																																																																																																																																																																																									//write json indexed field
		//printf ("printdochead OK!\n");
		//buff[textsize] ='\0';

		bwrite = write (fd, "<content>", 9);
		bwrite = write (fd, buff, textsize);
		bwrite = write (fd, "</content>", 10);

		free(buff);
		free(metawords);
/*
		free (mvastr);
		free (jsonstr);
*/

		printdocfooter (fd);
//		printf ("printdocfooter  OK!\n");
		docID++;
		/*
		 *
		*/
		//test
/*
		int i;
		printf ("realfilename - %s\n", realfilename);
		printf ("realfilename size - %d\n buffer : \n", textsize);
		for (i = 0; i < textsize; i++)
		{
			printf ("%c", buff[i]);
		}
		printf ("\n");
*/
		bread = read (fdin, &c, 1);
	}
	printf ("chname = %s\n", chname);
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
	char *buff;
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

	fdout = open (tempfilename, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
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
 * put incoming file into channel in format filesize, real filename, json, file data
 */

void putfile2channel (char * inputchname, char * outputchname, char *realfilename, char *json)
{

/*
	if (tContentLength > FS_MAX_FILE_LENGTH)
		printf ("Too big file. File %s skiped\n", filename);
*/

	int fin, fout;
	fin = open (inputchname, O_RDONLY); // open input channel
	fout = open (outputchname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR); //open output channel
	if ((fin < 0) || (fout < 0))
	{
		printf("error open file (), fdin=%d, fdout=%d\n", fin, fout);
		return;
	}

	long fsize = getfilesize_fd (fin, NULL, 0);

	char ext[strlen (realfilename)];
	getext(realfilename, ext);


	char bMetaOnly = 0;
	if ((strncmp (ext, "txt", 3) == 0) && fsize > FS_MAX_TEXT_FILE_LENGTH)
	{
		printf("Too big txt file ( > 2MB). Skipping content indexing for file %s. Indexing meta tags only.\n", realfilename);
		bMetaOnly = 1;
	}

	else if ((strncmp (ext, "odt", 3) == 0 || strncmp (ext, "docx", 4) == 0 || strncmp (ext, "pdf", 3) == 0 || strncmp (ext, "doc", 3) == 0) &&  fsize > FS_MAX_FILE_LENGTH)
	{
		printf("Too big odt, doc, docx, pdf file ( > 10MB). Skipping content indexing for file %s. Indexing meta tags only.\n", realfilename);
		bMetaOnly = 1;
	}

	char *buff;
	char *headbuf = (char *) malloc (strlen (realfilename) + 20 + strlen (json));
	//check for max file length
	if (fsize > FS_MAX_TEXT_FILE_LENGTH)
	{
		sprintf (headbuf,"%d %s //%s ~~", 5, realfilename, json);
	}
	else
		sprintf (headbuf,"%d %s //%s ~~", (int) fsize, realfilename, json);

	buff = (char *) malloc (fsize + strlen (headbuf));
	long bwrite = 0;

	//= write (fout, headbuf, strlen (headbuf)); // write header of file -
	strncpy (buff, headbuf, strlen (headbuf));

	long bread = 0;
	//check for max file length
	if (fsize > FS_MAX_TEXT_FILE_LENGTH)
	{
		sprintf (buff + strlen (headbuf), "other");
		fsize = 5;
	}
	else
		bread = read (fin, buff + strlen (headbuf), fsize);

	bwrite += write (fout, buff, bread + strlen (headbuf));

	close (fin);
	close (fout);
	printf ("%ld bytes writed into file %s from file %s, realfilename %s\n", bwrite, outputchname, inputchname, realfilename);
	free (buff);
	return;
}

/*
 * put extracted text data to channel in format: size of text, real file name, extracted text
 */

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
		}
	}
	return filteredbuffsize;
}

void printstat (long mainbytes, long deltabytes, int filecount, const char * mainindexname, char *deltaindexname)
{
	float konvmainbytes = (mainbytes / 1024) > 0 ? (mainbytes / (1024 * 1024)) > 0 ? (mainbytes / (1024 * 1024 * 1024)) > 0 ? (mainbytes / (1024.0 * 1024.0 * 1024.0)) : (mainbytes / (1024.0 * 1024.0)) : (mainbytes / 1024.0) : mainbytes;
	float konvdeltabytes = (deltabytes / 1024) > 0 ? (deltabytes / (1024 * 1024)) > 0 ? (deltabytes / (1024 * 1024 * 1024)) > 0 ? (deltabytes / (1024.0 * 1024.0 * 1024.0)) : (deltabytes / (1024.0 * 1024.0)) : (deltabytes / 1024.0) : deltabytes;
	const char *strmain = (mainbytes / 1024) > 0 ? (mainbytes / (1024 * 1024)) > 0 ? (mainbytes / (1024 * 1024 * 1024)) > 0 ? "Gb" : "Mb" : "Kb" : "B";
	const char *strdelta = (deltabytes / 1024) > 0 ? (deltabytes / (1024 * 1024)) > 0 ? (deltabytes / (1024 * 1024 * 1024)) > 0 ? "Gb" : "Mb" : "Kb" : "B";

	printf ("%s = %ld, (%5.2f %s)\n", mainindexname, mainbytes, konvmainbytes, strmain);
	printf ("%s = %ld, (%5.2f %s)\n", deltaindexname, deltabytes, konvdeltabytes, strdelta);
	printf ("file count = %d\n", filecount);
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
		fdefile = open (readbuf, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
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
	int bread = 1;
	long deltabytes = 0;
	long mainbytes = 0;
	int filecount = 0;

	char blocksizestr [10];
	size_t blocksize;
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
#ifdef TEST
		printf("\nfile name length %s\n", blocksizestr);
#endif
		// read file name
		bread = read (fdinfile, readbuf, blocksize);
		readbuf [blocksize] = '\0';
#ifdef TEST
		printf("filename %s\n", readbuf);
#endif
		// получение количества байт в сохраненном файле
		bread = read(fdinfile, blocksizestr, 10);
		blocksizestr[10] = '\0';
#ifdef TEST
		printf("filesize %s\n", blocksizestr);
#endif
		blocksize = atoi (blocksizestr);

		size_t filelen = 0;
		filelen = blocksize;

		int fdefile;
		fdefile = open (readbuf, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
		if (fdefile <= 0)
		{
			printf ("*** ZVM Error open %s file for write. Increasing the current offset of the file descriptor on %zu bytes  and skip save this file.\n", readbuf, filelen);
			// add check type of device. If we can use lseek
			bread = lseek (fdinfile, filelen, SEEK_CUR);
			continue;
		}

		// readind and save data file
		char *buff = NULL;
		int blockreadsize = READWRITEBUFFSIZE;
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

#ifdef TEST
		else
			printf ("*** ZVM unpack file %s (%d bytes)  - OK!\n", readbuf, writeb);
#endif
	}
#ifdef TEST
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char* ) DELTAINDEX);
#endif
	close (fdinfile);
}

/*
 ZVM Function for packing all index files from directory in ZeroVM FS, specified in Zsphinx.conf to /dev/output device
*/

void newbufferedpack (char *devname, char *dirname)
{
#ifdef TEST
	printf("*** ZVM (bufferedpackindexfd_) start pack index to %s \n", devname);
#endif
	int fdpackfile;

	fdpackfile = open (devname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);


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
					//bread = read (fd, buff, blocksize);
					bytecount += bread;
					bwrite = write(fdpackfile, buff, bread);
				}
			} else
				bytecount = 0;

			close (fd);
#ifdef TEST
			printf ("file %s (%d bytes) packed - OK!\n", newpath, (int) size);
#endif
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
#ifdef TEST
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char *) DELTAINDEX);
	printf ("*** ZVM pack completed successfully!\n");
#endif
}

void bufferedpackindexfd (char * devname)
{
#ifdef TEST
	printf("*** ZVM (bufferedpackindexfd_) start pack index to %s \n", devname);
#endif
	int fdpackfile;

	fdpackfile = open (devname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);


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
