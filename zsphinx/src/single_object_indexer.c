/*
 * Single_object_indexer.c
 *
 *  Created on: Mar 30, 2014
 *      Author: Volodymyr Varchuk
 */


//#include "../../bzip2_.h"
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "lists.h"
#include "types_.h"
#include "fileutil.h"
#include "xml_util.h"
#include <zlib.h>
#include <dirent.h>
#include <libgen.h>


//extractors
#include "../../antiword-0.37/main_u_.h"
#include "../../zxpdf-3.03/xpdf/pdftotext_.h"
#include "../../docxextract/docxtotext_.h"

char *get_text_from_file ( char *, size_t *);


void mylistdir_ (char *path)
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
		printf ("%s/%s D_TYPE = %d\n",path, entry->d_name, entry->d_type);
		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				strcpy (newpath, path);
				len = strlen (newpath);
				if (newpath [len-1] != '/')
					strcat (newpath, "/");
				strcat (newpath, entry->d_name);
				mylistdir_ (newpath);
			}
		}
	}
	closedir(dir);
}

unsigned long int num_CRC32 (const char * str)
{
	unsigned long int crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, str, strlen(str));
	return crc;
}

char * str_CRC32 (const char * str)
{
	unsigned long int crc = crc32(0L, Z_NULL, 0);
	char *	buff = NULL;
	crc = crc32(crc, str, strlen(str));
	buff = (char *) malloc (sizeof (char) * 20);
	sprintf( buff, "%zu", crc );
	return buff;
}

int setFileTypeFilter ( SingleList_t *pList )
{
	initList( pList );
	addToList( "txt", pList );
	addToList( "doc", pList );
	addToList( "docx", pList );
	addToList( "pdf", pList );
	addToList( "odt", pList );
	addToList( "sh", pList );
	return 0;
}

int do_extract_text (char *input_file, char *output_file)
{
	char *ext = NULL;
	ext = (char *) getext_( input_file );

	if ( strcasecmp( ext, "doc" ) == 0)
	{
		int ret = doc_to_text (input_file,  output_file );
	}
	else if (( strcasecmp( ext, "txt" ) == 0) || ( strcasecmp( ext, "sh" ) == 0) )
	{
		output_file [ strlen ( output_file ) - 4 ] = '\0';
	}
	else if ( strcasecmp( ext, "docx" ) == 0)
	{
		docx_to_text( input_file, output_file );
	}
	else if ( strcasecmp( ext, "odt" ) == 0)
	{
		docx_to_text( input_file, output_file );
	}
	else if ( strcasecmp( ext, "pdf" ) == 0)
	{
		pdf_to_text( input_file, output_file );
	}
	else
	{
		free (ext);
		return 1;
	}
	free (ext);
	return 0;
}

char *get_text_from_file ( char * file_name, size_t *text_size  )
{
	struct stat st;
	size_t fsize, bread, i = 0;
	int fd = 0;
	char *buff = NULL;

	int ret_stat = stat ( file_name, &st );
	fsize = st.st_size;

	buff = (char *) malloc ( sizeof ( char ) * ( fsize + 10 ) );

	fd = open ( file_name, O_RDONLY );
	if ( fd <= 0)
	{
		printf ( "Cannot open %s file\n", file_name );
		return NULL;
	}
	bread = read (fd, buff, fsize);
	close (fd);

	*text_size = bread;

	return buff;
}

int filtering_buff ( char *buff, size_t buff_size )
{
	size_t i = 0;
	for ( i = 0; i < buff_size; i++)
	{
		if ( !isalnum( ( char * ) buff[i] )  )
			buff[i] = ' ';
	}
	return 0;
}

int add_doc_to_xml (int xml_fd, char *fileName)
{
	struct stat st;
	struct tm *t;
	int ret = 0;
	char *xml_ = NULL;
	char *text = NULL;
	char *fileLenBuff = NULL;
	char *tmpFile = NULL;
	size_t size_text = 0;


	if ((ret = stat(fileName , &st))!=0)
    {
		fprintf(stderr, "stat failure error .%d", ret);
		return -1;
    }

	fileLenBuff  = (char *) malloc ( sizeof ( char ) * ( 25 ) );
	sprintf ( fileLenBuff, "%zu", st.st_size );

	tmpFile = (char *) malloc ( sizeof (char) * ( strlen ( fileName ) + 10) );
	sprintf ( tmpFile, "%s.tmp" , fileName );

	if (do_extract_text ( fileName, tmpFile ) == 1)
	{
		text = NULL;
		size_text = 0;
	}
	else
	{
		text = get_text_from_file( tmpFile, &size_text);
		filtering_buff ( text, size_text );
	}

	open_xml_document_( xml_fd, num_CRC32( fileName ) );
	write_XML_Elemet_Size( xml_fd, "CONTENT_FIELD", text, size_text );
	write_XML_Elemet_( xml_fd, "PATH_INFO_FIELD", fileName );
	write_XML_Elemet_( xml_fd, "PATH_INFO", fileName );
	write_XML_Elemet_( xml_fd, "TIMESTAMP", "");
	write_XML_Elemet_( xml_fd, "CONTENT_LENGTH", fileLenBuff );
	close_xml_document_( xml_fd );

	free (fileLenBuff);
	free (tmpFile);
	free ( text );

	return 0;
}



int docs_to_xml (char * path)
{
	int i = 0;
	SingleList_t tFileList, *pFileList=&tFileList, tFileTypeFilter, *pFileTypeFilter = &tFileTypeFilter;
	int xml_fd;
	initList( pFileList );
	setFileTypeFilter ( pFileTypeFilter );
	get_file_list ( path, pFileList, pFileTypeFilter );

//	get_list_from_file ( pFileList );

	for ( i = 0; i < pFileList->count; i++)
	{
		printf ( "%d. %s \n", i, pFileList->list[i] );
	}
///////////////////////
	xml_fd = open_xml_( XML_PATH ); //FIXME const
	for ( i = 0; i < pFileList->count; i++)
	{
		add_doc_to_xml ( xml_fd, pFileList->list[i] );
	}
	close_xml_( xml_fd );
///////////////////////

	freeList( pFileList );
	freeList( pFileTypeFilter );
	return 0;
}

int save_settings_to_fs ()
{
	if ( mkdir( "/settings", 0777 ) != 0 )
	{
		fprintf ( stderr, "Indexer settings not loaded\n" );
		return -1;
	}

	int conf = copy_to_file_from_fd (0, SPHINX_CONFIG_FILE );
	if ( conf <=0 )
	{
		fprintf ( stderr, "Indexer settings not loaded\n" );
		return -1;
	}
	return 0;
}

int save_object_to_fs ()
{
	//
	char *real_obj_path = NULL;
	char *real_obj_name = NULL;
	int obj  = 0;
	int fd_dev = 0;

	real_obj_path = getenv ( PATH_INFO_NAME );
	if ( !real_obj_path )
		return -1;
	// detect obj type

	real_obj_name = basename ( real_obj_path );
	if ( !real_obj_name )
		return -1;

	fd_dev = open ( OBJECT_DEVICE_NAME, O_RDONLY );
	if ( fd_dev <= 0 )
		return -1;

	obj = copy_to_file_from_fd (fd_dev, real_obj_name );
	if ( obj <=0 )
	{
		fprintf ( stderr, "Indexer settings not loaded\n" );
		return -1;
	}

	return 0;
}

int do_index_xml ()
{
	char *ind_argv[] = {"indexer", "--config", SPHINX_CONFIG_FILE, SPHINX_INDEX_NAME};
	int ind_argc = sizeof (ind_argv) / sizeof (char *);

	if ( mkdir( "/index", 0777 ) != 0 )
	{
		fprintf ( stderr, "Index directory not created\n" );
		return -1;
	}

	int ret_ind = indexer_main(ind_argc, ind_argv);
	return ret_ind;
}

int save_index ()
{
	rename ( SPHINX_CONFIG_FILE, "/index/sphinx.conf" );
	newbufferedpack_ ( INDEX_SAVE_PATH, "/index" );
	return 0;
}

int prepare_object ()
{
	if ( prepare_temp_dir ( TEMP_DIR ) < 0 )
		return -1;

	if ( save_object_to_fs() < 0 )
		return -1;

	extractfile ( basename ( getenv( PATH_INFO_NAME ) ) );

	return 0;
}

int main (int argc, char ** argv )
{

	if ( save_settings_to_fs () < 0 )
		return -1;

	if ( prepare_object () < 0 )
		return -1;

	docs_to_xml( TEMP_DIR );
	print_file( XML_PATH );
	do_index_xml ();
	save_index ();


	return 0;
}
