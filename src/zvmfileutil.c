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
	const char *str = "</content>\n</sphinx:document>\n \n";
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

		buff = (char *) malloc (textsize + 1);
		bread = read (fdin, buff, textsize);
//		printf ("bread = %d\n", bread);

		/*
		 * write to xml pipe readed data
		*/

		printdochead (fd, realfilename, docID);
//		printf ("printdochead OK!\n");
		//buff[textsize] ='\0';
		int bwrite;
		bwrite = write (fd, buff, textsize);
		free(buff);
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


	// FIX if an incorrect format of the read data
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

	if (fsize >= MAX_FILE_LENGTH)
	{
		printf ("Too big file\n");
		return;
	}
	char *buff;
	char *headbuf = (char *) malloc (strlen (realfilename) + 20);
	sprintf (headbuf,"%d %s //", (int) fsize, realfilename);
	buff = (char *) malloc (fsize + strlen (headbuf));
	long bwrite = 0;
	//= write (fout, headbuf, strlen (headbuf)); // write header of file -
	strncpy (buff, headbuf, strlen (headbuf));
	long bread = read (fin, buff + strlen (headbuf), fsize);
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
	printf ("*** ZVM (unpackindexfd) start unpack from %s\n", devname);
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
			printf ("*** ZVM unpack index completed successfully!\n");
			close (fdinfile);
			printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char*) DELTAINDEX);
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
			int tellsize = lseek (fdinfile, 0, SEEK_CUR);
			int bytesleft = 0;
			printf ("Error *\n");
/*
			while ( (bread = read (fdinfile, &c, 1)) > 0)
			{
				//printf ("%c", c);
				bytesleft++;
			}
*/
			printf("*** ZVM Error format packfile c=*%c*, current position = %d, bytes left = %d\n", c, tellsize, bytesleft);
			close (fdinfile);
			printstat (mainbytes, deltabytes, filecount, (char*) MAININDEX, (char *) DELTAINDEX);
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
		else
			printf ("*** ZVM unpack file %s (%d bytes)  - OK!\n", readbuf, writeb);
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
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char* ) DELTAINDEX);
	close (fdinfile);
}

/*
 ZVM Function for packing all index files from directory in ZeroVM FS, specified in Zsphinx.conf to /dev/output device
*/

void bufferedpackindexfd (char * devname)
{
	printf("*** ZVM (bufferedpackindexfd_) start pack index to %s \n", devname);
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
	printf ("blocksize = %d\n", blocksize);
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
//			if (bread < 0)
//			{
//				close (fd);
//				continue;
//			}
//			bwrite = write (fdpackfile, buff, size);
			//bwrite = write (1, buff, size);
			sprintf (tempstr, "{%s}\n", newpath);
			bwrite = write (fdpackfile, tempstr, strlen (tempstr));
			//bwrite = write (1, tempstr, strlen (tempstr));
			close (fd);
			printf ("file %s (%d bytes) packed - OK!\n", newpath, (int) size);


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
	printstat (mainbytes, deltabytes, filecount, (char *) MAININDEX, (char *) DELTAINDEX);

	printf ("*** ZVM pack completed successfully!\n");
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
		printf ("%s/%s\n",path, entry->d_name);
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
