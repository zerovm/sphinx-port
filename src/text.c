#include <stdio.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <dirent.h>
#include <string.h>


void printheader (FILE *f)
{
	char *externalfilename = getenv("fname");
	//printf ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // заголовок XML файла, генерируется в xmlpipecreator
	//printf ("<sphinx:document id=\"%d\">\n", docID); // заголовок документа в потоке, генерируется в xmlpipecreator
	fprintf (f, "<filename>%s</filename>\n", (char) externalfilename); //
	fprintf (f, "<content>\n");

}

void printfooter (FILE *f)
{
	  fprintf (f, "</content>\n");
	  fprintf (f, "</sphinx:document>\n \n");
}

void mylistdir (char *path)
{
  	DIR *dir;
	struct dirent *entry;
	//struct stat sb;
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
	}
	closedir(dir);
}

#undef ZVMDEBUG



int main (int argc, char *argv[]) 
{
//	char *oldlocale;
//	oldlocale = setlocale (LC_ALL, "ru_RU.UTF-8");
//	printf ("set locale %s\n", oldlocale);
	
	FILE *fin = argc == 2 ? fopen (argv[1], "r"): stdin;
	FILE *fout = argc == 2 ? stdout : fopen ("/dev/out/xmlpipecreator", "w");
#ifdef ZVMDEBUG
	if (argc == 2)
		mylistdir ("/home/volodymyr/tf");
	else
		mylistdir ("/");
#endif
	char wc;
	int charcount = 0;
	int totalcharcount = 0;
	printheader (fout);
	while ((wc = (char)fgetc(fin)) != EOF) {
		totalcharcount++;
		if (isalnum (wc) || isspace(wc)) {
			//wprintf (L"size of wc %d\n", sizeof(wc));
			charcount++;
			fprintf (fout, "%c", wc);
		}
		else
			fprintf (fout, "%c", ' ');
	}
	printfooter (fout);
	printf ("\n*** extracted symbols %d filtered %d  \n",charcount, totalcharcount);//, getenv["fname"]);
	fflush (NULL);
	if (fout != stdout)
		fclose (fout);
	return 0;
}
