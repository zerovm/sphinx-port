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
 * return size of file in bytes
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
* write to indexer file documet header for sphinx xml format
*/

void printdochead (int fd, char *realfilename, int docID)
{
	char str[strlen (realfilename) + 50];
	int bwrite;

	sprintf (str, "<sphinx:document id=\"%d\">\n", docID);
	bwrite = write (fd, str, strlen (str));

	sprintf (str, "<filename>%s</filename>\n<content>\n", realfilename);
	bwrite = write (fd, str, strlen (str));
	return;
}


/*
* write to indexer file documet header for sphinx xml format
*/

void printdocfooter (int fd)
{
	char *str = "</content>\n</sphinx:document>\n \n";
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
	while ( bread > 0 )
	{
		char textsizebuff [50];
		int textsizebufflen;
		char realfilename [2048];
		int realfilenamelen;

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
		//read text data.
		printf ("read data\n");
		char *buff;
		printf ("malloc\n");
		buff = (char *) malloc (textsize + 1);
		bread = read (fdin, buff, textsize);
		printf ("bread = %d\n", bread);

		/*
		 * write to xml pipe readed data
		*/

		printdochead (fd, realfilename, docID);
//		printf ("printdochead OK!\n");
		//buff[textsize] ='\0';
		int bwrite;
		bwrite = write (fd, buff, textsize);
		printf ("try free\n");
//		free(buff);
//		buff = 0;
		printf ("free\n");
//		printf ("write OK!\n");
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
	if (!tempfilename)
	{
		printf ("*** ZVM Error generate temp filename\n");
		return fmap;
	}

	if (!chname)
	{
		printf ("*** ZVM Error open in channelname\n");
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

	blockreadsize = 1024*1024;
	totalreadsize = 0;
	readcount = 0;
	buffsize = blockreadsize;
	totalreadsize = 0;
	bread = 0;
	buff = (char *) malloc (blockreadsize);
	while ((bread = read (fdin, buff + readcount * blockreadsize, blockreadsize)) > 0)
	{
		buffsize += blockreadsize;

		resizebuff = (char *) realloc ((char *)buff, buffsize);

		if (resizebuff != NULL && resizebuff != buff)
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

	// FIX if an incorrect format of the read data
	i = 0;
	while (isdigit (buff[i]))
		nbytes [i] = buff[i++];
	nbytes [i] = '\0';

	realfilesize = atoi (nbytes);

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
	int bwrite = write (fdout, buff + i, realfilesize);
	fmap.realfilesize = bwrite;
	close (fdin);
	close (fdout);
	//free (buff);
	//buff = 0;
	return fmap;
}


/*
 * put incoming file into channel in format filesize, real filename, file data
 */

void putfile2channel (char * inputchname, char * outputchname, char *realfilename)
{
	int fin, fout;
	fin = open (inputchname, O_RDONLY);
	fout = open (outputchname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if ((fin < 0) || (fout < 0))
	{
		printf("error open file (), fdin=%d, fdout=%d\n", fin, fout);
		return;
	}
	long fsize = getfilesize_fd (fin, NULL, 0);
	char buff[fsize];
	char *headbuf = (char *) malloc (strlen (realfilename) + 20);
	sprintf (headbuf,"%d %s //", (int) fsize, realfilename);
	long bwrite = write (fout, headbuf, strlen (headbuf)); // write header of file -
	long bread = read (fin, buff, fsize);
	bwrite += write (fout, buff, bread);
	close (fin);
	close (fout);
	printf ("%ld bytes writed into file %s from file %s, realfilename %s\n", bwrite, outputchname, inputchname, realfilename);
	return;
}

/*
 * put extracted text data to channel in format: size of text, real file name, extracted text
 */

int puttext2channel (char *bufftext, long size, char *realfilename, int channelfd)
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
	//*filteredbuff = (char *) malloc (sizeof (buff) + 1);
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

void unpackindexfd (char * devname)
{
	//

	printf ("*** ZVM start unpack from %s\n", devname);

	int fdinfile;


	//FILE *infile;
	char dirName[] = "index";
	int result=mkdir(dirName, 0755);
	if (result != 0)
		printf ("*** ZVM Error can`t create directory %s\n", dirName);
	//char devname []  = "/dev/input"; ZVM
	//char devname []  = "/dev/input";
	int MAXREAD = 1024;
	//char indexfilename[MAXREAD];
	char readbuf [MAXREAD];
	int letcount;
	letcount = 0;
//	infile = fopen (devname, "r");

	fdinfile = open (devname, O_RDONLY);

	if (fdinfile <= 0)
	{
		printf ("*** ZVM error input indexpack file\n");
		return;
	}


//	if (!infile)
//	{
//		printf ("*** ZVM error input indexpack file\n");
//		return;
//	}
	int c;
	int filelen;
	filelen = 0;
	int bread;
	bread = 1;
	while (bread > 0)
	{
		// получение количества байт сохраненном файле в файле
		letcount = 0;
		//c = getc (infile);
		bread = read (fdinfile, &c, 1);
		if (bread == 0)
		{
			printf ("*** ZVM unpack index completed successfully!\n");
			//fclose (infile);
			close (fdinfile);
			return;
		}

		if (c != '{')
		{
			printf("*** ZVM Error format packfile\n");
			//fclose (infile);
			close (fdinfile);
			return;
		}
		bread = read (fdinfile, &c, 1);
		while (!isspace(c) && bread >=0) {
			readbuf[letcount++] = c;
			bread = read (fdinfile, &c, 1);
		}
		readbuf [letcount] = '\0';
		filelen = atoi (readbuf);
		//получение имени файла
		letcount =0;
		bread = read (fdinfile, &c, 1);
		while (c != '\n')
		{
			readbuf [letcount++] = c;
			bread = read (fdinfile, &c, 1);
		}
		readbuf [--letcount] = '\0';
		//FILE *efile;
		int fdefile;
		//efile = fopen (readbuf, "w");
		fdefile = open (readbuf, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
//		if (!efile)
//		{
//			printf ("*** ZVM Error open %s file for write\n", readbuf);
//			fclose (infile);
//			return;
//		}
		if (fdefile <= 0)
		{
			printf ("*** ZVM Error open %s file for write\n", readbuf);
			close (fdinfile);
			return;

		}
		char *buff = (char *) malloc (filelen);
//		int readb = fread(buff,sizeof (char),filelen,infile);
//		int writeb = fwrite(buff,sizeof (char),filelen,efile);
		int readb = read(fdinfile, buff,filelen);
		int writeb = write(fdefile, buff,readb);

//		fflush (efile);
		close (fdefile);
		if (readb != writeb)
			printf ("*** Warning while unpacking index file, readb = %d, writeb = %d\n", readb, writeb);
		else
			printf ("*** ZVM unpack file %s (%d bytes)  - OK!\n", readbuf, writeb);
		bread = read (fdinfile, &c, 1);
		while (c != '\n')
			bread = read (fdinfile, &c, 1);
	}
	close (fdinfile);
}

/*
 ZVM Function for packing all index files from directory in ZeroVM FS, specified in Zsphinx.conf to /fdev/output device
  */

void bufferedpackindexfd (char * devname)
{
	printf("*** ZVM start pack index to %s \n", devname);
//	FILE *packfile;
	int fdpackfile;


	//char packfilename[]= "/dev/output"; // ZVM
	//char packfilename[]= "/dev/input";
//	packfile = fopen (devname, "w");
	fdpackfile = open (devname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);

//	if (!packfile)
//	{
//		printf ("*** ZVM Error open packfile (write)%s\n", devname);
//		return;
//	}

	if ( fdpackfile  <= 0 )
	{
		printf ("*** ZVM Error open packfile (write)%s\n", devname);
		return;
	}

	char indexpath[]="index"; // deirectory with  index files and zspfinx.conf
  	DIR *dir;
	struct dirent *entry;
	dir = opendir(indexpath);
	char *newpath;

	if (!dir)
		printf ("*** ZVM Error open DIR %s\n", indexpath);
	int bytecount;
	bytecount = 0;
	while(entry = readdir(dir))
	{

		//printf ("item - index/%s\n", entry->d_name);
		if(entry->d_type != DT_DIR)
		{
			newpath = (char *) malloc (strlen (entry->d_name) + strlen(indexpath) + 2);
			sprintf(newpath, "%s/%s", indexpath, entry->d_name);

			int c;
//			FILE *f;
//			f = fopen(newpath, "rb");
			long size;
			long buffsize;
//			fseek(f, 0, SEEK_END);
//			size = ftell(f);
//			rewind (f);

			int fd;
			fd = open (newpath, O_RDONLY);
			size = getfilesize_fd(fd, NULL, 0);

			//fprintf (packfile, "{%d %s}\n", (int) size, newpath);



			buffsize = size + strlen (newpath) * 2 + 50;
			//printf ("buffsize = %ld\n", buffsize);
			char *buff;
			buff = (char *) malloc (buffsize);

			sprintf (buff, "{%d %s}\n", (int) size, newpath);

			long bread;
			int tempbufflen;
			tempbufflen = strlen (buff);
			//printf ("tempbufflen = %ld\n", tempbufflen);
			bread = read (fd, buff + tempbufflen, size);
			if (bread < 0)
			{
				close (fd);
				continue;
			}
			//printf ("Read OK!\n, tempbufflen + size = %ld\n", tempbufflen + size);

			sprintf (buff + tempbufflen + size, "{%s}\n", newpath);
			long bwrite;
			bwrite = write (fdpackfile, buff, tempbufflen + size + strlen (newpath) + 3);
			//free (buff);


//			while (!feof (f))
//			{
//				c = getc (f);
//				if (c != EOF)
//					putc (c, packfile);
//			}
//			fprintf (packfile, "{%s}\n", newpath);
//			fclose (f);
			close (fd);
			free (buff);
			printf ("file %s (%d bytes) packed - OK!\n", newpath, (int) size);
		}
	}
//	fflush (packfile);
//	fclose (packfile);
	close (fdpackfile);
	printf ("*** ZVM End pack index OK!\n");
}
