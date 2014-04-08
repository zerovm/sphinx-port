/*
 * xml_util.c
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "xml_util.h"

int open_xml_ ( char *XML_file )
{
	char *XML_open =
"<?xml version=\"1.0\" encoding=\"utf-8\"?> \n\
<sphinx:docset>\n\
<sphinx:schema>\n\
<sphinx:field name=\"CONTENT_FIELD\" type=\"string\" />\n\
<sphinx:field name=\"PATH_INFO_FIELD\" type=\"string\" />\n\
<sphinx:attr name=\"PATH_INFO\" type=\"string\" />\n\
<sphinx:attr name=\"TIMESTAMP\" type=\"timestamp\" />\n\
<sphinx:attr name=\"CONTENT_LENGTH\" type=\"int\" />\n\
</sphinx:schema>\n\
\n\
</sphinx:docset>\n";
	int fd = 0;
	int bwrite = 0;
	fd = open ( XML_file, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );
	if (fd <= 0)
	{
		fprintf (stderr, "error open %s file", XML_file);
		return -1;
	}
	bwrite = write ( fd, XML_open, strlen ( XML_open ) );
	return fd;
}

int close_xml_ (int fd)
{
	char *XML_close = "</sphinx:docset>";
	int bwrite = 0;
	bwrite = write ( fd, XML_close, strlen ( XML_close ) );
	close ( fd );
	return 0;
}

int open_xml_document_ ( int fd, unsigned long int docID )
{
	char *XML_ = "<sphinx:document id=\"%zu\">\n";
	char XML[strlen ( XML_ ) + 25];
	int bwrite = 0;
	sprintf ( XML, XML_, docID );
	bwrite = write ( fd, XML, strlen ( XML ) );
	return 0;
}

int close_xml_document_ (int fd)
{
	char *XML = "</sphinx:document>\n\n";
	int bwrite = 0;
	bwrite = write ( fd, XML, strlen ( XML ) );
	return 0;
}

int write_XML_Elemet_ (int fd, char *elemeNtname, char *element)
{
	int bwrite = 0;
	int strSize = 50; //stock
	if (element != NULL)
		strSize += strlen(element) * 2;
	if (elemeNtname != NULL)
		strSize += strlen(elemeNtname);
	else
	{
		printf ("*** ZVM Error. Wrong element name\n");
		return 0;
	}
	char *str = (char*) malloc (sizeof (char) * strSize);
	sprintf (str, "<%s><![CDATA[%s]]></%s>\n", elemeNtname, element, elemeNtname);
	bwrite = write (fd, str, strlen (str));
	return bwrite;
}

int write_XML_Elemet_Size (int fd, char *elemeNtname, char *element, size_t e_size)
{
	int bwrite = 0;
	int strSize = 50; //stock
	char *temp = NULL;
	int temp_len = 0, temp_total_len = 0;

	if (element != NULL)
		strSize += strlen(element) * 2;
	if (elemeNtname != NULL)
		strSize += strlen(elemeNtname);
	else
	{
		printf ("*** ZVM Error. Wrong element name\n");
		return 0;
	}

	temp = (char *) malloc ( sizeof (char) * ( 50 + e_size + strlen(elemeNtname) * 2 ) );
	sprintf ( temp, "<%s><![CDATA[", elemeNtname );
	temp_len = strlen ( temp );
	memcpy( temp + temp_len, element, e_size );
	sprintf ( temp + temp_len + e_size, "]]></%s>\n", elemeNtname );
	temp_total_len = temp_len + e_size + strlen ( temp + + temp_len + e_size );

	//char *str = (char*) malloc (sizeof (char) * strSize);
	//sprintf (str, "<%s><![CDATA[%s]]></%s>\n", elemeNtname, element, elemeNtname);

	bwrite = write (fd, temp, temp_total_len);
	free( temp );

	return bwrite;
}

int do_XML ( char * XML_name )
{

	return 0;
}
