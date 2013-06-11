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

#define ZVMDEBUG
#undef ZVMDEBUG

void getworsfromfile ( char * );
char * getext (char * );
void myxmlpipe (char *);
void myxmlpipe2 (char *);
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
		if(entry->d_type == DT_DIR)
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
			
			getwordsfromfile (newpathf);
			docID++;

//			fprintf(myout,"%s\n", extfile);
			/*
			 * old version of xmlpipecreator
			 *
			if ((strcmp (extfile ,".txt") == 0) //|| (strcmp (extfile ,".conf") ==0))
				#ifdef USE_LIBEXPAT
					myxmlpipe2 (newpathf);
				#else
					myxmlpipe (newpathf);
				#endif
			*/

		}

	}
	closedir(dir);
}

void getwordsfromfile (char *filename)
{
	FILE *f;
/*	char *rulocale = "ru_RU.UTF-8";
	char *oldlocale = setlocale (LC_ALL, rulocale);

	if (!oldlocale)
		pritnf ("*** error set russian %s locale\n", rulocale);
	else
		wpritnf ("** locale set to %s \n", oldlocale);
*/
	int c;
	int pos;
	f = fopen (filename, "r");
	if (!f)
		return;

	while ( !feof (f) )
	{
		c= (char) getc(f);
		if (c != EOF)
			putc(c, myout);
	}
	fclose (f);
}


// заголовок XML потока
void createxmlpipe (void)
{
	//
	//char *xmldochead = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<sphinx:docset>\n\
<sphinx:schema>\n\
<sphinx:field name=\"content\"/> \n\
<sphinx:attr name=\"filename\" type=\"string\"/> \n\
<sphinx:attr name=\"lastmodified\" type=\"timestamp\"/> \n\
<sphinx:attr name=\"lastaccess\" type=\"timestamp\"/> \n \
</sphinx:schema>\n\
\n";

	char *xmldochead = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<sphinx:docset>\n\
<sphinx:schema>\n\
<sphinx:field name=\"content\"/> \n\
<sphinx:attr name=\"filename\" type=\"string\"/> \n\
</sphinx:schema>\n\
\n";

//<sphinx:attr name=\"lastmodified\" type=\"timestamp\"/> \n\

	fout (xmldochead);
}
// footer of sphinx document in XML stream
void closexmlpipe (void)
{
	char *xmldocend = "</sphinx:docset>\n";
	fout (xmldocend);
}	

// не используется в варинате решения "один зеровм на один файл"
void myxmlpipe2 (char *filename)
{
	FILE *f;
	struct stat sb;
	char c;

	stat (filename, &sb);
	f = fopen (filename, "r");
	if (!f)
		return;
	
	char *text = malloc (strlen(filename) + 1024);// [1024];
	char docnum [1024];
	strcpy (text, "<sphinx:document id=\"");
	sprintf(docnum, "%d", doccount);
	strcat (text, docnum);
	strcat (text, "\">\n");
	fout (text); // doc id
//		<sphinx:attr name=\"filename\" type=\"string\"/> \n\

	strcpy (text, "<filename>");
	strcat (text, filename);
	strcat (text, "</filename>\n");
	fout (text); // filename
	sprintf (docnum, "%d", sb.st_mtime);
	strcpy (text, "<lastmodified>");
	strcat (text, docnum);
	strcat (text, "</lastmodified>\n");
	fout (text); // lastmodified

	sprintf (docnum, "%d", sb.st_atime);
	strcpy (text, "<lastaccess>");
	strcat (text, docnum);
	strcat (text, "</lastaccess>\n");
	fout (text); // lastmodified

	strcpy (text, "<content>\n ![CDATA[");
	fout (text);
	while ((c=getc(f)) != EOF )
	{
//		if ((c != '<') && (c != '>') && (c != '&') && (c != '\') && ( c != '/') || (c >='0' && c <='9') || (c>='A' && c<='z') 				|| (c>='А' && c<='я') || (c == ' ')) 
		if (isalpha(c) || isalnum (c) || isspace(c) || (c== '\n'))
			fprintf (myout, "%c", c);	
	}
	strcpy (text, "\n]]</content>\n");
	fout (text);
	strcpy (text, "</sphinx:document>\n \n");
	fout (text);
	doccount++;
}

void myxmlpipe (char *filename)
{
	FILE *f;
	struct stat sb;
	int c;

	stat (filename, &sb);
	f = fopen (filename, "r");
	if (!f)
		return;

	char text [1024];
	char docnum [1024];
	strcpy (text, "<document>\n");
	fout (text); //

	sprintf(text, "<id>%d</id> \n", doccount);
	fout (text); // doc id

	sprintf(text, "<group>%d</group> \n", doccount);
	fout (text); // doc id


	sprintf (text, "<timestamp>%d</timestamp>\n", sb.st_mtime);
	fout (text); // lastmodified

	sprintf (text, "<title>%s</title>\n", filename);
	fout (text); // doc id

	strcpy (text, "<body>\n \n");
	fout (text);

	while ((c=getc(f)) != EOF )
	{
		if ((c != '<') && (c != '>') && (c != '&') && (c != '\\') && ( c != '/') || (c >='0' && c <='9') || (c>='A' && c<='z')|| (c>='А' && c<='я') || (c == ' '))
//		if (isalpha(c) || isalnum (c) || isspace(c) )
			fprintf (myout, "%c", c);
	}
	strcpy (text, "\n</body>\n");
	fout (text);
	strcpy (text, "</document>\n \n");
	fout (text);
	doccount++;
}

int main(int argc, char **argv)
{
	if (argc == 2)
		outf = stdout;
	else
		outf = fopen ("/dev/out/indexer","w");
	char c = '0';

	char p[] = "/dev/in";//malloc (strlen(argv[1]) + 2);
	//#ifndef USE_LIBEXPAT
	//	mylistdir (p);
	//#else
#ifdef ZVMDEBUG
	printf ("*** output device is %d \n", outf);
	printf ("*** start transfer to indexer\n");
#endif
	createxmlpipe ();
	mylistdir (p);
	closexmlpipe ();
#ifdef ZVMDEBUG
	printf ("*** transfer to indexer Complete!\n");
#endif
	//#endif
	fflush (outf);
	fclose (outf);
	return 0;
}
