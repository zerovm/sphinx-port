/*
 * fileutil.c
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */


#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "types_.h"
#include "lists.h"
#include "fileutil.h"

#define MAX_BUFF 1024

void reverse_ (char s[])
{
	int c, i, j;
	for (i = 0, j = strlen(s)-1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

char *getext_ (char * fname)
{
	int i = 0;
	int len = 0;
	char *buff = NULL;
	if ( fname == NULL )
		return NULL;
	len = strlen ( fname );
	for (i = len; i >= 0; i--)
	{
		if ( fname [i] == '.')
		{
			buff = (char *) malloc ( CHARSIZE(len - i + 1) );
			strncpy ( buff, fname + i + 1, len - i + 1);
			return buff;
		}
		if (fname [i] == '/')
		{
			return NULL;
		}
	}
	return NULL;
}

char *getext__ (const char *fname)
{
	if (fname == NULL)
		return NULL;
	int len = strlen (fname);
	int i;
	char *ext = NULL;
	i = 0;
	if (len == 0)
		return  NULL;

	ext = (char *) malloc ( sizeof (char) * strlen (fname) );
	while ((fname [len] != '.') && (fname [len] != '/') && len != 0)
	{
		if ((fname [len] != '\n') && (fname [len] != '\0'))
			ext[i++] = fname [len--];
		else
			len--;
	}
	ext[i] = '\0';
	printf("%s\n", ext);
	reverse_ (ext);
	return ext;
}

void get_list_from_file ( SingleList_t *list )
{
	char *fname = "/files.txt";
	char buff [MAX_BUFF];

	FILE *f;

	f = fopen ( fname, "r" );

	if ( !f )
	{
		printf ( "error open %s file\n", fname );
		return;
	}

	while ( fgets ( buff, MAX_BUFF, f ) != NULL )
	{
		printf ( "print buff %s  \n", buff );
		addToList( buff, list );
	}
	fclose(f);

	return;
}

int get_file_list (char *path, SingleList_t *pList, SingleList_t *pFileTypeList)
{
	static int a;
	DIR *dir;
	struct dirent *entry;
	dir = opendir(path);
	int len;
	if(dir == 0)
	{
		return -1;
	}
	while((entry = readdir(dir)))
	{

		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				char *newpath = NULL;
				newpath = (char *)  malloc ( sizeof ( char ) * ( strlen (entry->d_name) + strlen (path) + 2 ) );
				if ( strcmp (path, "/") == 0 )
					sprintf ( newpath, "/%s", entry->d_name );
				else
					sprintf ( newpath, "%s/%s", path, entry->d_name );
				get_file_list (newpath, pList, pFileTypeList);
				free (newpath);
			}
		}
		if ( entry->d_type == DT_REG )
		{
			char *filePath = NULL;
			filePath = (char *) malloc ( sizeof ( char ) * ( strlen (entry->d_name) + strlen (path) + 2 ) );
			sprintf( filePath, "%s/%s", path,  entry->d_name);
			int i = 0;
			char *fileExt = NULL;
			//fileExt = getext_( filePath );
/*
			for ( i = 0; i < pFileTypeList->count; i++)
				if ( strcmp ( fileExt , pFileTypeList->list[i]) == 0 )
				{
					char *freeFilePath = filePath;
					if ( filePath[0] == '/' && filePath[1] == '/' )
						filePath++;
					addToList( filePath, pList );
					free ( freeFilePath );
				}
*/
			char *freeFilePath = filePath;
			if ( filePath[0] == '/' && filePath[1] == '/' )
				filePath++;
			addToList( filePath, pList );
			free ( freeFilePath );
			//free (fileExt);
		}
	}
	closedir(dir);
	return 0;
}

int print_dir_tree (char *path)
{
//	static int a;
	DIR *dir;
	struct dirent *entry;
	dir = opendir(path);
	int len;
	if(dir == 0)
	{
		return -1;
	}
	while((entry = readdir(dir)))
	{
		if ( strcmp (path, "/") == 0 )
			printf ("/%s D_TYPE = %d\n", entry->d_name, entry->d_type);
		else
			printf ("%s/%s D_TYPE = %d\n", path, entry->d_name, entry->d_type);

		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				char *newpath = NULL;
				newpath = (char *)  malloc ( sizeof ( char ) * ( strlen (entry->d_name) + strlen (path) + 2 ) );
				if ( strcmp (path, "/") == 0 )
					sprintf ( newpath, "/%s", entry->d_name );
				else
					sprintf ( newpath, "%s/%s", path, entry->d_name );
				print_dir_tree (newpath);
				free (newpath);
			}
		}
	}
	closedir(dir);
	return 0;
}

void print_file (char *fileName)
{
	FILE *f;
	int bread = 0;
	char buff[ MAX_BUFF ];

	f = fopen ( fileName, "r" );
	if ( f == NULL )
	{
		printf ("Cannot open file %s\n", fileName);
		return;
	}
	else
		printf ("%s - OK\n", fileName);

	while ( (bread = fread ( buff, 1, MAX_BUFF, f )) > 0 )
	{
		fwrite ( buff, 1, bread, stdout );
	}
	fclose (f);
}

int copy_to_file_from_fd (int fd_in, char *filename )
{
	int data_size = 0;
	char buff [ MAX_BUFF ];
	int bread = 0, bwrite = 0;

	int fd_out;
	fd_out = open ( filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );
	if (fd_out <= 0)
		return 0;
	while ( ( bread = read ( fd_in, buff, MAX_BUFF ) ) > 0 )
	{
		bwrite = write ( fd_out, buff, bread );
		data_size += bwrite;
	}
	close (fd_out);
	return data_size;
}

void newbufferedpack_ (char *devname, char *dirname)
{

	int fdpackfile;

	fdpackfile = open (devname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
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
	int blocksize = 1024 * 64; // 10 Mb

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
					bytecount += bread;
					bwrite = write(fdpackfile, buff, bread);
				}
			} else
				bytecount = 0;

			close (fd);

			filecount++;
		}
	}
	free (buff);
	close (fdpackfile);
}

int prepare_temp_dir (char *dir_name)
{
	if (mkdir ( dir_name, 0777 ) != 0 )
		return -1;
	return 0;
}

int check_dir_exist (char * dir_path )
{
	struct stat st;
	int err = stat(dir_path, &st);
	if(-1 == err) {
		if(ENOENT == errno) {
			return 0;
		} else {
			perror("stat");
			exit(1);
		}
	} else {
		if(S_ISDIR(st.st_mode)) {
			return 1;
		} else {
			return 2;
		}
	}
}

char * get_file_name_without_ext ( char * file_name )
{
	char *base_file_name = NULL;
	char *file_name_without_ext = NULL;
	int basename_len = 0, i = 0, file_name_without_ext_len = 0;

	base_file_name = basename ( file_name );
	basename_len = strlen ( base_file_name );

	for ( i = basename_len; i > 0; i-- )
	{
		if ( base_file_name[i] == '.' )
			break;
	}
	file_name_without_ext_len = basename_len - (basename_len - i);
	file_name_without_ext = (char *) malloc( sizeof ( char ) * ( file_name_without_ext_len + 1 ) );
	memcpy( file_name_without_ext, base_file_name, file_name_without_ext_len );
	file_name_without_ext[file_name_without_ext_len] = '\0';
	return file_name_without_ext;
}







