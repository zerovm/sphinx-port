/*
 * zvmfileutil.h
 *
 *  Created on: Jul 8, 2013
 *      Author: Varchuk Volodymyr
 */

#ifndef ZVMFILEUTIL_H_
#define ZVMFILEUTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define MAXFIENAME 1024
#define MAX_FIELD_NAME_LENGTH 1024


#define DEVOUTNAME "/dev/out/xmlpipecreator"
#define CHECK_INDEXER_XML_DEV_NAME "/dev/in/indexer"
#define CHECK_INDEXER_IND_DEV_NAME "/dev/out/xmlpipecreator"

#define SERVERSOFT "zerocloud"

#define DOC_DEVICE_IN "/dev/in"
#define DOC_PREFIX ""
#define DOC_TEMP_DOC_FILE_NAME "temp.doc"

#define PDF_DEVICE_IN "/dev/in"
#define PDF_PREFIX ""
#define PDF_TEMP_PDF_FILE_NAME "temp.pdf"
#define PDF_TEMP_TXT_FILE_NAME "temp.txt"

#define OTHER_DEVICE_IN "/dev/in"
#define OTHER_PREFIX ""

#define TEXT_SNIPPET_SIZE 256

#define FS_DEVINPUTDATA "/dev/input" // file sender input channel
#define FS_MAX_TEXT_FILE_LENGTH 1024 * 1024 * 2
#define FS_MAX_FILE_LENGTH 1024 * 1024 * 10

#define I_DEVINPUTDATA "/dev/input"// indexer input
#define I_DEVOUTPUTDATA "/dev/output" // indexer output

#define EX_SEARCH_MODE_INPUT "/dev/input"
#define EX_READ_WRITE_SIZE 1024 * 64

#define S_DEVINPUTDATA "/dev/input" // search input
#define S_DEVOUTPUTDATA "/dev/output" // search output
#define DELTAINDEX "deltaindex"
#define MAININDEX "mainindex"
#define READWRITEBUFFSIZE 1024*64

#define INDEXDIRNAME "index"
#define ZIPBASED_TEMP_FILEANME "/temp/temp.zip"

#define ZLOGTIT "***ZVMLog"


#define PACKET_NUMBER_BLOCK_SIZE 10

#undef TEST

#include <string.h>

struct filemap {
	char realfilename[MAXFIENAME];
	char tempfilename[MAXFIENAME];
	char *json; // null-terminated string that contains the json data
	long realfilesize;
};

struct fileTypeInfo {
	char *sExt;
	char *sChannelname;
	int iFileType;
	size_t tFileSize;
	int iExtractorType;
	int bSaveFile;
	int bPlainText;
};

struct p_options {
    int bToOutput;
    int bTextSearchMode;
    size_t tStart;
    size_t tEnd;
    char *sWords;
    int iMaxWordsLen;
};

struct field_list
{
	char **fields;
	char **types;
	int fieldCount;
};

extern char *blank_attr_list[];
extern char *blank_attr_list_types[];
extern char *blank_field_list[];
extern char *blank_field_list_types[];

extern int blank_field_count;
extern int blank_attr_count;

int getZVMLogLevel ();

#define LOG_SERVER_SOFT 																	\
	if (getZVMLogLevel() > 0) 																\
	{																						\
		printf ("***ZVMLOG [LogLevel>0] [serversoft], \t%s\n", serversoft);					\
	}


#define LOG_NODE_NAME 																		\
	if (getZVMLogLevel() > 0)																\
	{																						\
		printf ("***ZVMLOG [LogLevel>0] [nodename], \t%s\n", argv[0]);						\
	}

#define LOG_ZVM(message, valuename, valuetype, value, level) 											\
		if (getZVMLogLevel () >= level)																	\
		{																								\
				printf("%s [LogLevel=%d] [%s], \t%"valuetype" \n", message, level, valuename, value);	\
		}

#define MY_MAX(A, B) strlen (A) >= strlen (B) ? strlen (A) : strlen (B)
#define MY_STRNCMP(A, B) strncmp (my_strtolower(A) ,my_strtolower(B), MY_MAX(my_strtolower(A), my_strtolower(B)))
//strncmp (my_strtolower(new_field_name) ,my_strtolower(fl.fields[i]), MY_MAX(my_strtolower(new_field_name), my_strtolower(fl.fields[i])))

void filesender2extractor (char *, char *, char *, char *, int);
struct filemap extractorfromfilesender (char *, char *);
char *getTextByHits (char *text, unsigned int, unsigned int);
size_t SaveFileFromInput (char *, char **);
struct fileTypeInfo checkMAxFileSize (char *, size_t);
struct p_options getOptions (int, char **);
char *generateJson (char **);
char *generateMetaWords (char *);
char *getTextByWords (char *, char *);
void PrintSnippet (char *, char *, unsigned int, unsigned int);
char *toLower (char*);
void sendConfigOK ();
char *my_strtolower (char *);
char *myrealloc (char *, size_t *);

void reverse (char *);
void getext (const char *, char *);
unsigned long getfilesize_fd (int, char *, int );
struct filemap getfilefromchannel (char *, char *);
void putfile2channel (char *, char *, char *, char *);
int puttext2channel (char *, long , char *, char *, int);
int getdatafromchannel (int, char *, int, struct field_list);
int getfilteredbuffer (const char *, long , char *);
void unpackindex_fd (char *);
void bufferedpackindexfd (char *);

void newbufferedunpack (char *);
void newbufferedpack (char *, char *);

int mymakedir (char *);
void mylistdir (char *);

#ifdef __cplusplus
}
#endif

#endif /* ZVMFILEUTIL_H_ */
