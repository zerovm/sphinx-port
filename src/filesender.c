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
#ifdef TEST
	char *devnamein = "/home/volodymyr/data_test/text.txt";
	char devnameout[1024];
	char *filename = "/home/volodymyr/data_test/text.txt";
	char ext[strlen (filename)];
	getext(filename, ext);
	printf ("*** ext = %s \n", ext);
	sprintf (devnameout, "/home/volodymyr/data_test/%s", ext);
	printf ("in - %s\n",devnamein);
	printf ("out - %s\n",devnameout);
#else
	char *devnamein = "/dev/input";
	char devnameout [50];
	char *filename = getenv("fname");
	char ext[strlen (filename)];
	getext(filename, ext);
	if (strncmp (ext, "txt", 3) == 0 || strncmp (ext, "odt", 3) == 0 || strncmp (ext, "docx", 4) == 0)
		sprintf (devnameout, "/dev/out/txt");
	else
		sprintf (devnameout, "/dev/out/%s", ext);
#endif
	putfile2channel (devnamein, devnameout, filename);
#ifdef TEST
	struct filemap fmap = getfilefromchannel ("/home/volodymyr/data_test/txt", "/home/volodymyr/data_test"); ///
	printf ("tmp name - %s; real file name - %s; size %d\n", fmap.tempfilename, fmap.realfilename, (int) fmap.realfilesize);
#endif
	return 0;
}
