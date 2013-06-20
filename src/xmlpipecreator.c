#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "../config/config.h"


#define myout outf
#define fout(st) fprintf (myout, "%s", st);
#define foutc(st) fprintf (myout, "%c", st);
#define USE_XMLPIPE2 0
#define MAXID_DEV_NAME "/dev/input"
#define DEV_OUTPUT_NAME "/dev/out/indexer"

#define pritnf printf // дурацкий макрос


#define ZVMDEBUG
#undef ZVMDEBUG

int getworsfromfile ( char * );
void mylistdir (char *);

FILE *outf;

int doccount=1000;
int doccount2=10000;
int docID=1;

void mylistdir (char *path) 
{
  	DIR *dir;
	struct dirent *entry;
	struct stat sb;
	char statcheck [1024];
	char newpath[1024];
	char newpathf[1024];
	char extfile [1024];
	dir = opendir(path);
	int len, lennew, lennewlast;
	if(dir == 0)
	{
		return;
	}
//	printf ("* %s", path);
	while(entry = readdir(dir))
	{	
		printf ("%s/%s\n",path, entry->d_name);
		if(entry->d_type == DT_DIR && (strcmp (entry->d_name, "input")) != 0)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				strcpy (newpath, path);
				len = strlen (newpath);
				if (newpath [len-1] != '/')
					strcat (newpath, "/");
				strcat (newpath, entry->d_name);
				mylistdir (newpath);
			}
		}
		else
		{
			strcpy (newpathf, path);
			len = strlen (newpathf);
			if (newpathf [len-1] != '/')
				strcat (newpathf, "/");
			strcat (newpathf, entry->d_name);
			lennew = strlen (newpathf);
			lennewlast = lennew;
			extfile[0] = '\n';
			int extcountsymb=0;
			int readscount;
			readscount = 0;
			
			while ((newpathf [lennew] != '.') && (newpathf [lennew] != '/') )
				lennew--;
			if (newpathf[lennew] == '/') 
				extfile[0] = '\n';
			if (newpathf[lennew] == '.')
			{	
				for (extcountsymb=0; lennew <= lennewlast; lennew++, extcountsymb++)
					extfile[extcountsymb] = newpathf[lennew];
				
			}
			// print to stdout content of file newpathf
			fprintf (myout, "<sphinx:document id=\"%d\">\n", docID); // doc id

			readscount = getwordsfromfile (newpathf);
			if (readscount == 0)
			{
				fprintf (myout, "<filename>empty_file</filename>\n");
				fprintf (myout, "<content>empty\n");
				fprintf (myout, "</content>\n");
				fprintf (myout, "</sphinx:document>\n\n");
			}
			docID++;
		}
	}
	closedir(dir);
}

void setmaxid (char *fname, int maxid)
{
	FILE *f;
	printf ("*** ZVM (setmaxid) set MaxID = %d, device = %s\n", maxid, fname);
	f = fopen (fname, "w");
	if (!f)
	{
		printf ("*** ZVM Error save maxID for document.\n");
		return;
	}
	fprintf (f, "%d", maxid);
	fclose (f);
	return;
}

int getmaxid (char *fname)
{

	FILE *f;
	f = fopen (fname, "r");
	//printf ("*** ZVM Try to open %s device.\n", fname);
	if (!f)
	{
		printf ("*** ZVM Warning. Error open file %s with maxID document. MaxID set to 1.\n", fname);
		setmaxid(MAXID_DEV_NAME, 1);
		return 1;
	}
	char maxidchar[20];
	int maxid;
	maxid = 0;
	int i;
	i = 0;
	int c;
	while ((c=(char)getc (f)) != EOF)
	{
		printf (" *** %c\n", c);
		if (isdigit(c))
			maxidchar[i++] = c;
	}
	maxidchar [i] = '\0';
	//printf ("*** ZVM 1. maxID  = %s\n", maxidchar);
	maxid = atoi (maxidchar);
	fclose (f);
	//pritnf ("*** ZVM 2. maxID =  %d\n", maxid);
	if (maxid <= 0)
	{
		maxid = 1;
		printf ("*** ZVM Wrong readed MaxID form %s. maxID set to 1.\n", fname);
		setmaxid (MAXID_DEV_NAME ,1);
	}
	printf ("*** ZVM maxid = %d\n", maxid);
	return maxid;
}

int getwordsfromfile (char *filename)
{
	FILE *f;
/*
 	char *rulocale = "ru_RU.UTF-8";
	char *oldlocale = setlocale (LC_ALL, rulocale);

	if (!oldlocale)
		pritnf ("*** error set russian %s locale\n", rulocale);
	else
		wpritnf ("** locale set to %s \n", oldlocale);
*/
	int c;
	int pos;
	int readcount;
	readcount =0;
	f = fopen (filename, "r");
	if (!f)
	{
		printf("*** ZVM Error open %s file\n");
		return 0;
	}

	while ( !feof (f) )
	{
		readcount++;
		c= (char) getc(f);
		if (c != EOF)
			putc(c, myout);
	}
	fclose (f);
	return readcount;
}


// заголовок XML потока
void createxmlpipe (void)
{
	char *xmldochead = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<sphinx:docset>\n\
<sphinx:schema>\n\
<sphinx:field name=\"content\"/> \n\
<sphinx:attr name=\"filename\" type=\"string\"/> \n\
</sphinx:schema>\n\
\n";

	fout (xmldochead);
}

// footer of sphinx document in XML stream
void closexmlpipe (void)
{
	char *xmldocend = "</sphinx:docset>\n";
	fout (xmldocend);
}	

void mylistdirtest (char *path)
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
	while(entry = readdir(dir))
	{
		printf ("%s/%s\n",path, entry->d_name);
		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				strcpy (newpath, path);
				len = strlen (newpath);
				if (newpath [len-1] != '/')
					strcat (newpath, "/");
				strcat (newpath, entry->d_name);
				mylistdirtest (newpath);
			}
		}

	}
	closedir(dir);
}

int main(int argc, char **argv)
{

	if (argc == 2 && (strcmp (argv[1], "--onlystdout") == 0))
		outf = stdout;
	else
		outf = fopen (DEV_OUTPUT_NAME ,"w");
	char c = '0';
	char p[] = "/dev/in";
#ifdef ZVMDEBUG
	printf ("*** output device is %d \n", outf);
	printf ("*** start transfer to indexer\n");
#endif
	docID = getmaxid (MAXID_DEV_NAME);
	printf ("*** ZVM start search incoming devices\n");
	createxmlpipe ();
	mylistdir (p);
	closexmlpipe ();
	setmaxid (MAXID_DEV_NAME, docID);
#ifdef ZVMDEBUG
	printf ("*** transfer to indexer Complete!\n");
#endif
	//#endif
	fflush (outf);
	fclose (outf);
	/*if (argc == 2)
		if (strcmp (argv[1],"--duplicate") == 0)
		{
			printf ("*** ZVM duplicate\n");
			//fflush (stdout);
			FILE *f;
			FILE *df;
			df = stdout;
			char buff[1024];
			char c;
			int bread;
			int bwrite;
			f = fopen (DEV_OUTPUT_NAME, "r");
			while (!feof (f))
			{
				//bread = fread (buff,1,1024,f);
				//bwrite = fwrite (buff,1,bread,df);
				c=getc (f);
				putc (c, df);
			}
			fclose (f);
		}*/
	printf ("*** ZVM xmlpipe - OK\n");
	return 0;
}
