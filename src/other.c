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

int main (int argc, char *argv[])
{

    struct filemap fmap;

    char *filename;
    char *chname; //
    int fdout;

    char *path = "/dev/in";
    char *prefix = "";
    long  byteswrite2text;

  	DIR *dir;
	struct dirent *entry;


    byteswrite2text = 0;

    fdout = open (DEVOUTNAME, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
    if (fdout < 0 )
    {
    	printf ("*** ZVM Error open % output device\n", DEVOUTNAME);
    	return 1;
    }

	dir = opendir(path);
	if (dir ==0)
	{
		printf ("*** ZVM Error read dir %s, %d\n", path, dir);
		return 1;
	}
	while(entry = readdir(dir))
	{
		int temp;
		int filteredbufflen = 0;
		if(entry->d_type != DT_DIR && (strcmp (entry->d_name, "input")) != 0)
		{
			int fdin;

			int tempwritebytes2channel =0;

			chname = malloc (strlen(path) + strlen (entry->d_name) + 2);

			sprintf (chname, "%s/%s", path, entry->d_name);
			printf ("start chname = %s\n", chname);
			fmap = getfilefromchannel(chname, prefix);
			free (chname);

			int tmpsize, ftmp;
			ftmp = open (fmap.tempfilename, O_RDONLY);

			tmpsize = getfilesize_fd(ftmp, NULL, 0);
			printf ("tmp file name = %s, size = %d, detected size = %d\n", fmap.tempfilename, fmap.realfilesize, tmpsize);
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

			printf ("filename = %s json %s\n", fmap.realfilename, fmap.json);
			tempwritebytes2channel = puttext2channel (filteredbuff, strlen (filteredbuff), fmap.realfilename, fmap.json, fdout);
		}
	}
	close (fdout);
	printf ("other - all OK!\n");

	return 0;
}
