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
#include "filesender_.h"
#include <unistd.h>

#undef TEST

extern char **environ;



int filesender_main (int argc, char *argv[])
{
	char *devnamein = FS_DEVINPUTDATA;
	char devnameout [50];

	char *serversoft = getenv (SERVER_SOFTWARE_NAME);
	char *filename = NULL;

	char *json = NULL;

	char *cContentLength = NULL;
	size_t tContentLength = 0;

	char bPlainText = 0; //
	int bToOutput = 0;
	int i = 0;

	fileTypeInfo_t fti;

	LOG_SERVER_SOFT;
	LOG_NODE_NAME;

	for (i = 0; i < argc; i++ )
		if (strcmp (argv[i], "--save") == 0)
		{
			bToOutput = 1;
		}

	if (serversoft)
	{
		if (strncmp (SERVERSOFT ,serversoft, strlen (serversoft)) == 0)
		{
			filename = getenv(PATH_INFO_NAME);

			cContentLength = getenv(CONTENT_LENGTH_NAME);
			tContentLength = atoi (cContentLength);
		}
		else
		{
			printf ("Can not detect type of cluster swift or local\n");
			return 1;
		}
	}
	else
	{
		filename = getenv(PATH_INFO_NAME);
		tContentLength = (size_t) getfilesize_fd(0, filename, 1 );
	}

	LOG_ZVM ("***ZVMLog", "content type", "d", bPlainText, 2);
	LOG_ZVM ("***ZVMLog", "content length", "zu", tContentLength, 1);
	LOG_ZVM ("***ZVMLog", "file name", "s", filename, 1);

	json = generateJson(environ);

	LOG_ZVM ("***ZVMLog", "json length", "ld", strlen (json), 2);
	LOG_ZVM ("***ZVMLog", "json", "s", json, 3);

	fti = checkMAxFileSize ();

	if (bToOutput == 1 )
		sprintf (devnameout, "/dev/output");
	else
		sprintf (devnameout, "%s", fti.sChannelname);
	if (Mode == single_operation)
		sprintf (devnameout, FILESENDER_SINGLE_MODE_OUTPUT_FILE);

	LOG_ZVM ("***ZVMLog", "output device", "s", devnameout, 1);

	filesender2extractor (devnamein, devnameout, filename, json, fti.bPlainText);


#ifdef TEST
	struct filemap fmap = getfilefromchannel ("/home/volodymyr/data_test/txt", "/home/volodymyr/data_test"); ///
	printf ("tmp name - %s; real file name - %s; size %d\n", fmap.tempfilename, fmap.realfilename, (int) fmap.realfilesize);
#endif

	LOG_ZVM ("***ZVMLog", "OK!", "s", "", 1);
	return 0;
}
