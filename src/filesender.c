/*
 * filesender.c
 *
 *  Created on: Jul 5, 2013
 *      Author: Volodymyr Varchuk
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zvmfileutil.h"

#undef TEST

int main (int argc, char *argv[])
{
	char *devnamein = FS_DEVINPUTDATA;
	char devnameout [50];

	char *serversoft = getenv ("SERVER_SOFTWARE");
	char *filename = NULL;

	printf ("serversoft=%s\n", serversoft);

	if (serversoft)
	{
		if (strncmp (SERVERSOFT ,serversoft, strlen (serversoft)) == 0)
		{
			printf ("path info=%s\n", getenv("PATH_INFO"));
			filename = getenv("PATH_INFO");
		}
		else
		{
			printf ("Can not detect type of cluster swift or local\n");
			return 1;
		}
	}
	else
	{
		printf ("fname=%s\n", getenv("fname"));
		filename = getenv("fname");
	}

	char ext[strlen (filename)];
	getext(filename, ext);
	if (strncmp (ext, "txt", 3) == 0 || strncmp (ext, "odt", 3) == 0 || strncmp (ext, "docx", 4) == 0)
		sprintf (devnameout, "/dev/out/txt");
	else
		sprintf (devnameout, "/dev/out/%s", ext);
	putfile2channel (devnamein, devnameout, filename);
#ifdef TEST
	struct filemap fmap = getfilefromchannel ("/home/volodymyr/data_test/txt", "/home/volodymyr/data_test"); ///
	printf ("tmp name - %s; real file name - %s; size %d\n", fmap.tempfilename, fmap.realfilename, (int) fmap.realfilesize);
#endif
	return 0;
}
