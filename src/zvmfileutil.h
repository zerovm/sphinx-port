/*
 * zvmfileutil.h
 *
 *  Created on: Jul 8, 2013
 *      Author: Varchuk Volodymyr
 */

#ifndef ZVMFILEUTIL_H_
#define ZVMFILEUTIL_H_

#define MAXFIENAME 1024

#define DEVOUTNAME "/dev/out/xmlpipecreator"


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



#define FS_DEVINPUTDATA "/dev/input" // file sender input channel
#define FS_MAX_TEXT_FILE_LENGTH 1024 * 1024 * 2
#define FS_MAX_FILE_LENGTH 1024 * 1024 * 10

#define I_DEVINPUTDATA "/dev/input"// indexer input
#define I_DEVOUTPUTDATA "/dev/output" // indexer output

#define S_DEVINPUTDATA "/dev/input" // search input
#define S_DEVOUTPUTDATA "/dev/output" // search output
#define DELTAINDEX "deltaindex"
#define MAININDEX "mainindex"
#define READWRITEBUFFSIZE 1024*1024
#define MAX_FILE_LENGTH 1024 * 1024 * 10 // max length of file of any format 10 MB

#define INDEXDIRNAME "index"
//#define INDEXDIRNAME "/home/volodymyr/temp/index"
#define ZIPBASED_TEMP_FILEANME "/temp/temp.zip"
//#define ZIPBASED_TEMP_FILEANME "/home/volodymyr/temp.zip"
//#define INDEXDIRNAME "f1"
//#define INDEXDIRNAME "/home/volodymyr/disk/f1"


#define PACKET_NUMBER_BLOCK_SIZE 10

//#define TEST
#undef TEST

struct filemap {
	char realfilename[MAXFIENAME];
	char tempfilename[MAXFIENAME];
	char *json; // null-terminated string that contains the json data
	long realfilesize;
};


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


void filesender2extractor (char *, char *, char *, char *);
struct filemap extractorfromfilesender (char *, char *);

void reverse (char *);
void getext (const char *, char *);
unsigned long getfilesize_fd (int, char *, int );
struct filemap getfilefromchannel (char *, char *);
void putfile2channel (char *, char *, char *, char *);
int puttext2channel (char *, long , char *, char *, int);
//int getdatafromchannel (int *, char * , long *);
int getdatafromchannel (int, char *, int);
int getfilteredbuffer (const char *, long , char *);
void unpackindex_fd (char *);
void bufferedpackindexfd (char *);

void newbufferedunpack (char *);
void newbufferedpack (char *, char *);

int mymakedir (char *);
void mylistdir (char *);

#endif /* ZVMFILEUTIL_H_ */
