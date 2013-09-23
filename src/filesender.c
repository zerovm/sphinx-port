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
#include <unistd.h>

#undef TEST

extern char **environ;

char *myrealloc (char * buff, size_t *buffsize)
{
	char *newbuff;
	char *oldbuff;
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

char * generateJson ()
{
	size_t jsonmaxsize = 1024; // initial lengt of JSON data
	size_t jsonsize = 0;
	char *json = (char *) malloc (sizeof (char) * jsonmaxsize);

	char *tagfilters [] = {
			"CONTENT_LENGTH",
			"CONTENT_TYPE",
			"HTTP_X_OBJECT_META",
			"HTTP_ETAG",
			"HTTP_X_TIMESTAMP",
			"PATH_INFO",
			"SERVER_PROTOCOL",
			"HTTP_ACCEPT_ENCODING"
	};

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
		int j = 0;
		for ( j = 0; tagfilters[j]; j++ )
		{
			if ( (pFilterOK = strstr (pKey, tagfilters[j])) != NULL )
			{
				iFind = 1;
			}
		}
		if (iFind == 0)
		{
			free (pKey);
			free (pVal);
			continue;
		}

		size_t addsize = strlen (pKey) +  strlen (pVal) + 8; //8 symbols plus -------  1 '\t' + 1 '=' + 4 '"' + 1 ',' + 1 '\n'

		if ((jsonsize + addsize) > jsonmaxsize )
			json = myrealloc (json, &jsonmaxsize);

		sprintf (json + jsonsize + 1, "\t\"%s\":\"%s\",\n",pKey, pVal);
		jsonsize += addsize;
		free (pKey);
		free (pVal);
	}

	if (jsonsize + 2 > jsonmaxsize)
		json = myrealloc (json, &jsonmaxsize);
	sprintf (json + jsonsize - 1, "\n}");
	jsonsize += 2;
	return json;
}



int main (int argc, char *argv[])
{
	char *devnamein = FS_DEVINPUTDATA;
	char devnameout [50];

	char *serversoft = getenv ("SERVER_SOFTWARE");
	char *filename = NULL;

	char *json = NULL;

	char *cContentLength = NULL;
	size_t tContentLength = 0;

	char bPlainText = 0;

	printf ("serversoft=%s\n", serversoft);

	if (serversoft)
	{
		if (strncmp (SERVERSOFT ,serversoft, strlen (serversoft)) == 0)
		{
			printf ("path info=%s\n", getenv("PATH_INFO"));
			filename = getenv("PATH_INFO");

			cContentLength = getenv("CONTENT_LENGTH");
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
		printf ("fname=%s\n", getenv("fname"));
		filename = getenv("fname");

		tContentLength = (size_t) getfilesize_fd(0, filename, 1 );
	}

	json = generateJson();

	printf ("%s", json);

	char ext[strlen (filename)];
	getext(filename, ext);



	if ((strncmp (ext, "txt", 3) == 0 || strncmp (ext, "odt", 3) == 0 || strncmp (ext, "docx", 4) == 0) && tContentLength <= FS_MAX_TEXT_FILE_LENGTH)
		sprintf (devnameout, "/dev/out/txt");
	else if ((strncmp (ext, "pdf", 3) == 0 || strncmp (ext, "doc", 3) == 0) && tContentLength <= FS_MAX_FILE_LENGTH)
		sprintf (devnameout, "/dev/out/%s", ext);
	else
	{
		sprintf (devnameout, "/dev/out/other");
	}

	putfile2channel (devnamein, devnameout, filename, json);
#ifdef TEST
	struct filemap fmap = getfilefromchannel ("/home/volodymyr/data_test/txt", "/home/volodymyr/data_test"); ///
	printf ("tmp name - %s; real file name - %s; size %d\n", fmap.tempfilename, fmap.realfilename, (int) fmap.realfilesize);
#endif


	return 0;
}
