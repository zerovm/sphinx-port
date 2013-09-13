#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "zvmfileutil.h"
#include "../config/config.h"


#define myout outf
#define fout(st) fprintf (myout, "%s", st);
#define foutc(st) fprintf (myout, "%c", st);
#define USE_XMLPIPE2 0
#define MAXID_DEV_NAME_IN "/dev/input" // device for load document max ID
#define MAXID_DEV_NAME_OUT "/dev/output" // device for save document max ID
#define DEV_OUTPUT_NAME "/dev/out/indexer"

#define pritnf printf // дурацкий макрос

/*
#define ZVMDEBUG
#undef ZVMDEBUG
*/



FILE *outf;

int docID=1;

void mylistdir_xmlpipe (int fd, char *path)
{
  	DIR *dir;
	struct dirent *entry;
	dir = opendir(path);
	if(dir == 0)
	{
		return;
	}
	while(entry = readdir(dir))
	{	
		printf ("%s/%s\n",path, entry->d_name);
		if(!(entry->d_type == DT_DIR && (strcmp (entry->d_name, "input")) != 0))
		{
			char devname [strlen (path) + strlen (entry->d_name) + 5];
			sprintf (devname, "%s/%s", path, entry->d_name);
			int doccount;
			doccount = getdatafromchannel (fd, devname, docID);
			printf ("read and write OK %s/%s\n",path, entry->d_name);
			docID = doccount;
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
		setmaxid(MAXID_DEV_NAME_OUT, 1);
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
		//printf (" *** %c\n", c);
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
		setmaxid (MAXID_DEV_NAME_OUT ,1);
	}
	printf ("*** ZVM maxid = %d\n", maxid);
	return maxid;
}

// заголовок XML потока
void createxmlpipe (int fd)
{
	char *xmldochead = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<sphinx:docset>\n\
<sphinx:schema>\n\
<sphinx:field name=\"content\"/> \n\
<sphinx:attr name=\"filename\" type=\"string\"/> \n\
<sphinx:attr name=\"meta\" type=\"json\"/> \n\
</sphinx:schema>\n\
\n";
	int bwrite;
	printf ("write header of xml document\n");
	bwrite = write (fd, xmldochead, strlen (xmldochead));
	printf ("%d bytes header writed \n", bwrite);
	return;
}

// footer of sphinx document in XML stream
void closexmlpipe (int fd)
{
	char *xmldocend = "</sphinx:docset>\n";
	int bwrite;
	bwrite = write (fd, xmldocend, strlen (xmldocend));
}	

int main(int argc, char **argv)
{
	char c = '0';
	char p[] = "/dev/in";
	int fd;
	fd = open (DEV_OUTPUT_NAME, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if (fd <= 0)
	{
		printf ("*** ZVM. Error open %s deivce\n", DEV_OUTPUT_NAME);
		return 1;
	}
#ifdef ZVMDEBUG
	printf ("*** output device is %d \n", outf);
	printf ("*** start transfer to indexer\n");
#endif
	docID = getmaxid (MAXID_DEV_NAME_IN);
	printf ("*** ZVM start search incoming devices\n");
	createxmlpipe (fd);
	mylistdir_xmlpipe (fd, p);
	closexmlpipe (fd);
	close (fd);
	setmaxid (MAXID_DEV_NAME_OUT, docID);
#ifdef ZVMDEBUG
	printf ("*** transfer to indexer Complete!\n");
#endif
	//fflush (outf);
	//fclose (outf);
	printf ("*** ZVM xmlpipe - OK\n");
	return 0;
}
