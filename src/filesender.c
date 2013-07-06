/*
 * filesender.c
 *
 *  Created on: Jul 5, 2013
 *      Author: Volodymyr Varchuk
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


#define TEST

void reverse (char *);

long getfilesize_fd (int fd)
{
	long fsize;
	fsize = lseek (fd, 0, SEEK_END);
	lseek (fd, 0, SEEK_SET);
	return fsize;
}

char * getext (char * fname, char * ext)
{
	int len = strlen (fname);
	//char ext[len];
	int i;
	i = 0;
	if (len == 0)
		return NULL;

	while ((fname [len] != '.') && (fname [len] != '/') && len != 0)
	{
		if ((fname [len] != '\n') && (fname [len] != '\0'))
			ext[i++] = fname [len--];
		else
			len--;
	}
	ext[i] = '\0';
	reverse (ext);
	return ext;
}

void reverse (char *s)
{
	char temp[strlen(s)];
	int i, len;
	len = strlen (s);
	for (i=0; i < len; i++)
	{
		temp[i] = s[len - i - 1];
	}
	temp[len] = '\0';
	for (i = 0; i < len; i++);
		s[i] = temp [i];
}

int main (int argc, char *argv[])
{
	int fin, fout;

#ifdef TEST
	char *devnamein = "/home/volodymyr/data_test/text.txt";
	char devnameout[1024];
	char *filename = "/home/volodymyr/data_test/text.txt";
	char ext[strlen (filename)];
	sprintf (devnameout, "/home/volodymyr/data_test/%s", getext(filename, ext));
	printf ("in - %s\n",devnamein);
	printf ("out - %s\n",devnameout);
#else
	char *devnamein = "/dev/input";
	char devnameout [50];
	char *filename = getenv("fname");
	char ext[strlen (filename)];
	sprintf (devnameout, "/dev/out/%s", getext(filename, ext));
#endif

	fin = open (devnamein, O_RDONLY);
	fout = open (devnameout, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if ((fin < 0) || (fout < 0))
	{
		printf("error\n");
		return 1;
	}
	long fsize = getfilesize_fd (fin);
	char buff[fsize];
	char *headbuf = malloc (strlen (filename) + 20);

	sprintf (headbuf,"%d %s //", (int) fsize, filename);
	long bwrite = write (fout, headbuf, strlen (headbuf));
	long bread = read (fin, buff, fsize);
	bwrite = write (fout, buff, bread);

	close (fin);
	close (fout);
	return 0;
}
