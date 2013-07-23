/*
 * zvmfileutil.h
 *
 *  Created on: Jul 8, 2013
 *      Author: Varchuk Volodymyr
 */

#ifndef ZVMFILEUTIL_H_
#define ZVMFILEUTIL_H_

#define MAXFIENAME 1024

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
void unpackindexfd (char *);
void packindexfd (char *);
void bufferedpackindexfd (char *);

#endif /* ZVMFILEUTIL_H_ */
