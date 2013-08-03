/*
 * zvmfileutil.h
 *
 *  Created on: Jul 8, 2013
 *      Author: Varchuk Volodymyr
 */

#ifndef ZVMFILEUTIL_H_
#define ZVMFILEUTIL_H_

#define MAXFIENAME 1024

#define I_DEVINPUTDATA "/dev/output"
#define I_DEVOUTPUTDATA "/dev/output"

#define S_DEVINPUTDATA "/dev/input"
#define S_DEVOUTPUTDATA "/dev/output"
#define DELTAINDEX "deltaindex"
#define MAININDEX "mainindex"
#define READWRITEBUFFSIZE 1024*1024

#define INDEXDIRNAME "index"
#define ZIPBASED_TEMP_FILEANME "/temp/temp.zip"
//#define INDEXDIRNAME "f1"
//#define INDEXDIRNAME "/home/volodymyr/disk/f1"

#define TEST

struct filemap {
	char realfilename[MAXFIENAME];
	char tempfilename[MAXFIENAME];
	long realfilesize;
};

void reverse (char *);
void getext (const char *, char *);
long getfilesize_fd (int, char *, int );
struct filemap getfilefromchannel (char *, char *);
void putfile2channel (char *, char *, char *);
int puttext2channel (char *, long , char *, int);
//int getdatafromchannel (int *, char * , long *);
int getdatafromchannel (int, char *, int);
int getfilteredbuffer (const char *, long , char *);
void unpackindex_fd (char *);
void bufferedpackindexfd (char *);
int mymakedir (char *);

#endif /* ZVMFILEUTIL_H_ */
