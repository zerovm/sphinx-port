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

typedef enum {dev_stdin, dev_input} InputDevice_t;
typedef enum {txt, docx, odt, doc, rtf, pdf, other} Extractortype_t;
typedef enum {single_operation, cluster, none} OperationMode_t;



//swift metatags constant
#define PATH_INFO_NAME "PATH_INFO"
#define CONTENT_LENGTH_NAME "CONTENT_LENGTH"
#define CONTENT_TYPE_NAME "CONTENT_TYPE"
#define SERVER_SOFTWARE_NAME "SERVER_SOFTWARE"
//end swift metatags constant

//#define SINGLE_OPERATION_MODE

#define FILESENDER_SINGLE_MODE_OUTPUT_FILE "filesender.dat"
#define EXTRACTOR_SINGLE_MODE_OUTPUT_FILE "extractor.dat"
#define XML_SINGLE_MODE_OUTPUT_FILE "xml.dat"


#define MAXFIENAME 1024
#define MAX_FIELD_NAME_LENGTH 1024


#define DEVOUTNAME "/dev/out/xmlpipecreator"
#define CHECK_INDEXER_XML_DEV_NAME "/dev/in/indexer"
#define CHECK_INDEXER_IND_DEV_NAME "/dev/out/xmlpipecreator"

#define INDEXER_XML_INPUT_DEVICE_CLUSTER_MODE "/dev/in/xmlpipecreator"

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

extern OperationMode_t Mode;



struct filemap {
	char realfilename[MAXFIENAME];
	char tempfilename[MAXFIENAME];
	char *json; // null-terminated string that contains the json data
	long realfilesize;
};

typedef struct  {
	char *sExt;
	char *sChannelname;
	int iFileType;
	size_t tFileSize;
	Extractortype_t iExtractorType;
	int bSaveFile;
	int bPlainText;
} fileTypeInfo_t;

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

#define WRITE_TO_LOCAL_FILE_EXTRACTOR	int fd_save = 0;																\
																										\
fd_save = open ( EXTRACTOR_SINGLE_MODE_OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );	\
if (fd_save > 0)																										\
{																														\
	tempwritebytes2channel = puttext2channel (filteredbuff, filteredbufflen, fmap.realfilename, fmap.json, fd_save);	\
	close(fd_save);																										\
}

#define WRITE_TO_LOCAL_FILE_FILESENDER																					\
int 																													\
fd_save = 0;																											\
fd_save = open ( FILESENDER_OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR );			\
if (fd_save > 0)																										\
{																														\
	filesender2extractor (devnamein, devnameout, filename, json, fti.bPlainText);										\
	close(fd_save);																										\
}																														\



void filesender2extractor (char *, char *, char *, char *, int);
struct filemap extractorfromfilesender (char *, char *);
char *getTextByHits (char *text, unsigned int, unsigned int);
size_t SaveFileFromInput (char *, char **, InputDevice_t);
fileTypeInfo_t checkMAxFileSize ();
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
long long int getfilesize_fd (int, char *, int );
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
