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
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

extern char **environ;

int iZVMLogLevel = 1;


int main (int argc, char *argv[])
{

	char *serversoft = getenv ("SERVER_SOFTWARE");

	LOG_SERVER_SOFT;
	LOG_NODE_NAME;

    struct filemap fmap;

    char *filename;
    char *chname; //
    char devoutname [strlen (DEVOUTNAME) * 2];
    int fdout;

    char *path = (char *) OTHER_DEVICE_IN;
    char *prefix = (char *) OTHER_PREFIX;
    long  byteswrite2text;
    int i = 0;
    int bTextPlain = 0;

  	DIR *dir;
	struct dirent *entry;

    byteswrite2text = 0;

	struct p_options popt;
    ////////////////////////////////////////
    // end program options
    ////////////////////////////////////////
	popt = getOptions(argc, argv);

	int tempwritebytes2channel =0;

	if (popt.bTextSearchMode == 1)
	{
		/// add check if big file
		sprintf (fmap.tempfilename, "%s/temp.tmp", prefix);
		fmap.realfilesize = SaveFileFromInput (fmap.tempfilename, environ);
		if (getenv ("PATH_INFO") != NULL)
			sprintf (fmap.realfilename, "%s", getenv ("PATH_INFO"));
		if (getenv ("CONTENT_TYPE") != NULL)
		{
			if (strstr (getenv("CONTENT_TYPE"), "text/plain" ) != NULL)
				bTextPlain = 1;
		}
		char *buff;
		int txtbufflen =0;
		if (bTextPlain == 1 && fmap.realfilesize <= FS_MAX_TEXT_FILE_LENGTH)
		{
			buff = (char *) malloc (fmap.realfilesize * sizeof (char) + 1);
			txtbufflen = getbufffromtxt (fmap.tempfilename , buff);
		}
		buff [txtbufflen] = '\0';
		PrintSnippet (buff, fmap.realfilename, popt.tStart, popt.tEnd);
	}
	else
	{
	    if (popt.bToOutput == 1)
	    	sprintf (devoutname, "%s", "/dev/output");
	    else
	    	sprintf (devoutname, "%s", DEVOUTNAME);


	    fdout = open (devoutname, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	    if (fdout < 0 )
	    {
	    	printf ("*** ZVM Error open % output device\n", devoutname);
	    	return 1;
	    }

		LOG_ZVM ("***ZVMLog", "output device", "s", devoutname, 1);

		dir = opendir(path);
		if (dir ==0)
		{
			printf ("*** ZVM Error read dir %s, %d\n", path, dir);
			return 1;
		}
		LOG_ZVM ("***ZVMLog", "incoming channel dir", "s", path, 1);
		while((entry = readdir(dir)))
		{
			int temp;
			int filteredbufflen = 0;
			if(entry->d_type != DT_DIR && (strcmp (entry->d_name, "input")) != 0)
			{
				int fdin;

				chname = malloc (strlen(path) + strlen (entry->d_name) + 2);

				sprintf (chname, "%s/%s", path, entry->d_name);
				LOG_ZVM ("***ZVMLog", "channel name", "s", chname, 1);

				fmap = extractorfromfilesender(chname, prefix);
				free (chname);
				LOG_ZVM ("***ZVMLog", "real file name", "s", fmap.realfilename, 1);

				int tmpsize, ftmp;
				ftmp = open (fmap.tempfilename, O_RDONLY);

				tmpsize = getfilesize_fd(ftmp, NULL, 0);
				LOG_ZVM ("***ZVMLog", "temp file name", "s", fmap.tempfilename, 1);
				LOG_ZVM ("***ZVMLog", "real file size", "ld", fmap.realfilesize, 1);
				close (ftmp);

				if (fmap.realfilesize <= 0)
				{
					printf ("*** Error fmap.realfilesize = %d\n", fmap.realfilesize);
					continue;
				}

				fdin = open (fmap.tempfilename, O_RDONLY);
				if (fdin <=0 )
				{
					printf ("*** Error fdin = %d \n", fdin);
					continue;
				}

				char *filteredbuff = "other"; //

				LOG_ZVM ("***ZVMLog", "json length", "d", strlen (fmap.json), 2);
				LOG_ZVM ("***ZVMLog", "json length", "s", fmap.json, 3);
				tempwritebytes2channel = puttext2channel (filteredbuff, strlen (filteredbuff), fmap.realfilename, fmap.json, fdout);
			}
		}
	}
	close (fdout);
	LOG_ZVM ("***ZVMLog", "all ok", "", "", 1);

	return 0;
}
