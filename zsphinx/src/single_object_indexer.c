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
#include <locale.h>


//extractors
//#include "../../antiword-0.37/main_u_.h"
#include "../../zxpdf-3.03/xpdf/pdftotext_.h"
#include "../../docxextract/docxtotext_.h"
#include "../../hypermail/src/hypermail.h"
#include "../../catdoc-0.94.4/src/catdoc.h"




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
	buff = (char *) malloc (CHARSIZE( 20 ));
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
	addToList( "html", pList );
	addToList( "c", pList );
	addToList( "rtf", pList );
	return 0;
}

int do_extract_text (char *input_file, char *output_file)
{
	char *ext = NULL;
	ext = (char *) getext_( input_file );

	if (ext == NULL)
		return 1;
	if ( (strcasecmp( ext, "doc" ) == 0) || (strcasecmp( ext, "rtf" ) == 0) )
	{
		char *argv_catdoc [] = {"catdoc", input_file};
		int argc_catdoc = PCHARSIZE( argv_catdoc );
		catdoc_main((int)argc_catdoc, argv_catdoc ); //
		//doc_to_text (input_file,  output_file ); // old doc extractor. antiword
	}
	else if (( strcasecmp( ext, "txt" ) == 0) || ( strcasecmp( ext, "sh" ) == 0) || ( strcasecmp( ext, "html" ) == 0) || ( strcasecmp( ext, "c" ) == 0) )
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

	buff = (char *) malloc ( CHARSIZE(fsize + 10) );

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
	for ( i = 0; i < buff_size ; i++)
	{
		if ( (buff[i] < 'A' || buff[i] > 'Z' ) && (buff[i] < 'a' || buff[i] > 'z' ) && buff[i] != ':' && (buff[i] < '0' || buff[i] > '9' ) )
		{
			buff[i] = ' ';
		}
	}
	return 0;
}

int get_one_file_from_zip ( char *zip_name, char *file_in_zip )
{
	return extractfile( zip_name, file_in_zip );
}

int add_doc_to_xml ( int xml_fd, char *fileName, Input_Obj_Type tMode )
{
	struct stat st;
	struct tm *t;
	int ret = 0;
	char *xml_ = NULL;
	char *text = NULL;
	char *fileLenBuff = NULL;
	char *tmpFile = NULL;
	char *file_name_in_fs = NULL;
	size_t size_text = 0;

	if ( tMode == zip_obj )
	{
		if ( get_one_file_from_zip ( OBJECT_DEVICE_NAME, fileName ) != 0 )
		{
			return -1;
		}
		file_name_in_fs = malloc( CHARSIZE( strlen ( fileName ) + strlen ( TEMP_DIR ) + 2) );
		sprintf ( file_name_in_fs, "%s/%s", TEMP_DIR, fileName );
	}
	else if ( tMode == mail_obj )
	{
		file_name_in_fs = fileName;
	}

	if ( ( ret = stat( file_name_in_fs, &st ) ) != 0 )
	{
		fprintf( stderr, "stat failure error %d\n", ret );
		return -1;
	}

	fileLenBuff = (char *) malloc( CHARSIZE(50) );
	sprintf( fileLenBuff, "%zu", st.st_size );

	tmpFile = (char *) malloc( CHARSIZE( strlen ( file_name_in_fs ) + 10) );
	sprintf( tmpFile, "%s.tmp", file_name_in_fs );


	if ( do_extract_text( file_name_in_fs, tmpFile ) == 1 )
	{
		text = NULL;
		size_text = 0;
	}
	else
	{
		text = get_text_from_file( tmpFile, &size_text );
		filtering_buff( text, size_text );
	}
	if ( tMode == zip_obj )
	{
		remove( file_name_in_fs );
		free (file_name_in_fs);
		write_doc_toxml( xml_fd, num_CRC32( fileName ), text, size_text, fileName, fileLenBuff );
	}
	else if ( tMode == mail_obj )
	{
		char *base_name_without_ext = NULL;
		char *temp_dir_name = NULL;
		char *temp_base_name = NULL;
		char *temp_file_name = NULL;
		char *temp_pos = NULL;
		char *temp_filepath = NULL;
		char *temp_message_id = NULL;
		char *temp_full_file_name = NULL;
		char *use_file = NULL;
		char *do_not_index[] = { "date", "index", "subject", "author", "attachment" };
		int do_not_index_count = PCHARSIZE(do_not_index);
		int i = 0;
		int skip_indexing = 0;
		base_name_without_ext = get_file_name_without_ext( fileName );
		for( i = 0; i < do_not_index_count; i++ )
		{
			if ( strcasecmp( base_name_without_ext, do_not_index[i] ) == 0 )
			{
				skip_indexing = 1;
				break;
			}
		}
		if ( skip_indexing != 0 )
		{
			free( fileLenBuff );
			free( tmpFile );
			free( text );
			return 0;
		}
		// BUG in glibc. we need to save fileName
		temp_filepath = strdup( fileName );
		temp_dir_name = dirname( temp_filepath );
		temp_base_name = basename( temp_dir_name );
		temp_pos = NULL;
		temp_pos = strstr( temp_base_name, "-" );

		char *from_field = NULL;
		char *subj_field = NULL;
		char *offset_attr = NULL;
		char *sent_ch = NULL;
		char *recv_ch = NULL;
		time_t sent_tm_t;
		time_t recv_tm_t;
		char *message_id_attr = NULL;

		if ( temp_pos != NULL ) // if attachment
		{
			temp_pos++;
			temp_file_name = (char *) malloc( CHARSIZE( strlen ( TEMP_DIR ) + strlen ( temp_base_name ) + 20) );
			sprintf( temp_file_name, "%s/%s.html", TEMP_DIR, temp_pos );
			temp_message_id = get_message_ID_from_html( temp_file_name );
			temp_full_file_name = (char *) malloc( CHARSIZE( strlen (temp_message_id) + strlen ( fileName ) + 5 ) );
			sprintf( temp_full_file_name, "%s/%s", temp_message_id, fileName );
			use_file = temp_file_name;
		}
		else // if message body
		{
			temp_full_file_name = get_message_ID_from_html( fileName );
			use_file = fileName;
		}

		from_field = get_element_from_html( use_file, "email" );
		subj_field = get_element_from_html( use_file, "subject" );
		offset_attr = get_element_from_html( use_file, "offset_email" );
		sent_ch = get_element_from_html( use_file, "isosent" );
		recv_ch = get_element_from_html( use_file, "isoreceived" );
		message_id_attr = get_element_from_html( use_file, "id" );

		sent_tm_t = iso_to_secs( sent_ch );
		recv_tm_t = iso_to_secs( recv_ch );

		sprintf( sent_ch, "%ld", sent_tm_t );
		sprintf( recv_ch, "%ld", recv_tm_t );

		if ( subj_field != NULL )
			filtering_buff( subj_field, strlen ( subj_field ) );
		if ( from_field != NULL )
			filtering_buff( from_field, strlen( from_field ) );


		write_Email_message_to_xml( xml_fd, num_CRC32( temp_full_file_name ), text, size_text, from_field, subj_field, offset_attr, sent_ch, recv_ch,
				strchr( fileName, '/' ), message_id_attr );

		free( from_field );
		free( subj_field );
		free( offset_attr );
		free( sent_ch );
		free( recv_ch );
		free( message_id_attr );
		if ( temp_file_name )
			free( temp_file_name );
		free( temp_full_file_name );
	}
	free( fileLenBuff );
	free( tmpFile );
	free( text );

	return 0;
}


int get_message_info_from_html ()
{

	return 0;
}

int docs_to_xml (char * path, Input_Obj_Type tMode )
{
	int i = 0;
	SingleList_t tFileList, *pFileList=&tFileList, tFileTypeFilter, *pFileTypeFilter = &tFileTypeFilter;
	int xml_fd;
	char *ext = NULL;
	initList( pFileList );
	setFileTypeFilter ( pFileTypeFilter );

#ifndef OLD_ZRT
	if ( strcmp ( getext_( path ), "zip") == 0 )
	{
		get_file_list_inzip ( path , pFileList );
	}
	else
#endif
	if ( tMode == zip_obj )
		get_file_list_inzip( OBJECT_DEVICE_NAME, pFileList );
	else if ( tMode == mail_obj )
		get_file_list( path, pFileList, pFileTypeFilter );
	else
		return 0;

/*
	printf ( "list of indexed docs\n" );
	for ( i = 0; i < pFileList->count; i++)
		printf ( "%s\n", pFileList->list[i] );
*/

///////////////////////

	xml_fd = open_xml_( XML_PATH, tMode ); //FIXME const
	for ( i = 0; i < pFileList->count; i++)
	{
		add_doc_to_xml ( xml_fd, pFileList->list[i], tMode );
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

char * save_object_to_fs ()
{
	char *real_obj_path = NULL;
	char *real_obj_name = NULL;
	int obj  = 0;
	int fd_dev = 0;

	real_obj_path = getenv ( PATH_INFO_NAME );
	if ( !real_obj_path )
		return NULL;
	real_obj_name = basename ( real_obj_path );
	if ( !real_obj_name )
		return NULL;

	fd_dev = open ( OBJECT_DEVICE_NAME, O_RDONLY );
	if ( fd_dev <= 0 )
		return NULL;

	obj = copy_to_file_from_fd (fd_dev, real_obj_name );
	if ( obj <=0 )
	{
		fprintf ( stderr, "Indexer settings not loaded\n" );
		return NULL;
	}
	return real_obj_name;
}

int do_index_xml ()
{
	char *ind_argv[] = {"indexer", "--config", SPHINX_CONFIG_FILE, SPHINX_INDEX_NAME};
	int ind_argc = PCHARSIZE(ind_argv);

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
	rename ( SPHINX_CONFIG_FILE, "/index/zsphinx.conf" );
	newbufferedpack_ ( INDEX_SAVE_PATH, "/index" );
	return 0;
}

Input_Obj_Type get_work_mode (int argc, char ** argv)
{
	int i = 0;
	for ( i = 0; i < argc; i++ )
	{
		if ( (strcasecmp( argv [i], "--zip" )) == 0 || (strcasecmp( argv [i], "-z" )) == 0 )
		{
			return zip_obj;
		}
		else if ( (strcasecmp( argv [i], "--mbox" )) == 0 || (strcasecmp( argv [i], "-m" )) == 0 )
		{
			return mail_obj;
		}
	}
	return error_type;
}


int prepare_zip ()
{
 	return 0;//extractfile ( OBJECT_DEVICE_NAME, NULL );
}

int prepare_mbox ()
{
	char *argv_mbox [] = { "hypermail", "-m", OBJECT_DEVICE_NAME, "-d", TEMP_DIR };
	int argc_mbox = PCHARSIZE(argv_mbox);
	return main_mbox( argc_mbox, argv_mbox );
}

char * prepare_object ( Input_Obj_Type tMode )
{
	int is_zip = 0;
	int i = 0;

	if ( prepare_temp_dir ( TEMP_DIR ) < 0 )
		return NULL;

	if ( tMode == zip_obj )
	{
		if (prepare_zip () != 0 )
			return NULL;
	}
	else if ( tMode == mail_obj )
	{
		if (prepare_mbox() != 0)
			return NULL;
	}
	return TEMP_DIR;
}

char buff_stdin [0x1000];
char buff_stdout [0x1000];

int main (int argc, char ** argv )
{

	setvbuf(stdin, buff_stdin, _IOFBF, 0x1000);
	setvbuf(stdout, buff_stdout, _IOFBF, 0x1000);

	char *real_obj_name = NULL;
	Input_Obj_Type tMode;

	tMode = get_work_mode (argc, argv);

	if ( save_settings_to_fs () < 0 )
		return -1;

	if ( ( real_obj_name = prepare_object (tMode)) == NULL )
	{
		return -1;
	}
	docs_to_xml( TEMP_DIR, tMode );
	do_index_xml ();
	save_index ();

	//mylistdir( "/" );

	return 0;
}
