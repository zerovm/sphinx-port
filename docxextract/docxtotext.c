/*
   miniunz.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
//#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
//#define FTELLO_FUNC(stream) ftello64(stream)
//#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../src/zvmfileutil.h"


#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <utime.h>
#endif


#include "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)
#define FILENAME "file.docx"
#define DEVOUTNAME "/dev/out/xmlpipecreator"

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

/*
 *  ZVM
 * */

int dt; //dt - type of document

/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(filename,dosdate,tmu_date)
    const char *filename;
    uLong dosdate;
    tm_unz tmu_date;
{
#ifdef _WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
#ifdef unix || __APPLE__
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

int mymkdir(dirname)
    const char* dirname;
{
    int ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif unix
    ret = mkdir (dirname,0775);
#elif __APPLE__
    ret = mkdir (dirname,0775);
#endif
    return ret;
}

int makedir (newdir)
    char *newdir;
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
        if (buffer==NULL)
        {
                printf("Error allocating memory\n");
                return UNZ_INTERNALERROR;
        }
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}

void do_banner()
{
    printf("MiniUnz 1.01b, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at http://www.winimage.com/zLibDll/unzip.html\n\n");
}

void do_help()
{
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract crypted file using password\n\n");
}

void Display64BitsSize(ZPOS64_T n, int size_char)
{
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  int offset=19;
  int pos_string = 19;
  number[20]=0;
  for (;;) {
      number[offset]=(char)((n%10)+'0');
      if (number[offset] != '0')
          pos_string=offset;
      n/=10;
      if (offset==0)
          break;
      offset--;
  }
  {
      int size_display_string = 19-pos_string;
      while (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

int do_list(uf)
    unzFile uf;
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    for (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[256];
        unz_file_info64 file_info;
        uLong ratio=0;
        const char *string_method;
        char charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        if (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is crypted */
        if ((file_info.flag & 1) != 0)
            charCrypt='*';

        if (file_info.compression_method==0)
            string_method="Stored";
        else
        if (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            if (iLevel==0)
              string_method="Defl:N";
            else if (iLevel==1)
              string_method="Defl:X";
            else if ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        else
        if (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        else
            string_method="Unkn. ";

        Display64BitsSize(file_info.uncompressed_size,7);
        printf("  %6s%c",string_method,charCrypt);
        Display64BitsSize(file_info.compressed_size,7);
        printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}


int do_extract_currentfile(uf,popt_extract_without_path,popt_overwrite,password)
    unzFile uf;
    const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* password;
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    uLong ratio=0;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    //printf ("filename=%s \n", filename_inzip);

   	if ((strcmp (filename_inzip, "word/document.xml") != 0) && dt == 1)
   		return UNZ_OK;
   	if ((strcmp (filename_inzip, "content.xml") != 0) && dt == 2)
   		return UNZ_OK;

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    if (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            if (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        if (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}


int do_extract(uf,opt_extract_without_path,opt_overwrite,password)
    unzFile uf;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
{
    uLong i;
    unz_global_info64 gi;
    int err;
    FILE* fout=NULL;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int do_extract_onefile(uf,filename,opt_extract_without_path,opt_overwrite,password)
    unzFile uf;
    const char* filename;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
{
    int err = UNZ_OK;
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) == UNZ_OK)
        return 0;
    else
        return 1;
}

void wprintheader (FILE *f)
{
	char *externalfilename = getenv("fname");
	//printf ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // заголовок XML файла, генерируется в xmlpipecreator
	//printf ("<sphinx:document id=\"%d\">\n", docID); // заголовок документа в потоке, генерируется в xmlpipecreator
	fwprintf (f, L"<filename>%s</filename>\n", (wchar_t) externalfilename); //
	fwprintf (f, L"<content>\n");

}

void printheader (FILE *f)
{
	char *externalfilename = getenv("fname");
	//printf ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // заголовок XML файла, генерируется в xmlpipecreator
	//printf ("<sphinx:document id=\"%d\">\n", docID); // заголовок документа в потоке, генерируется в xmlpipecreator
	fprintf (f, "<filename>%s</filename>\n", externalfilename); //
	fprintf (f, "<content>\n");

}

void sprintheader (FILE *f, char *s)
{
	char *externalfilename = getenv("fname");
	sprintf (s, "<filename>%s</filename>\n", externalfilename); //
	sprintf (s, "<content>\n");

}


void wprintfooter (FILE *f)
{
	  fwprintf (f, L"</content>\n");
	  fwprintf (f, L"</sphinx:document>\n \n");
}

void printfooter (FILE *f)
{
	  fprintf (f, "</content>\n");
	  fprintf (f, "</sphinx:document>\n \n");
}



void writebuffer2channel ()
{

	return;
}

/*
 * takes the text between the text docx tags, fills the buffer with the filtered text. Return size of filtered text in buffer,
 * */

int gettextfromxml (char *filteredbuff)
{
	//
	int fdin; // input file descriptor
	char *finname = "document.xml"; // input file name

	fdin = open (finname, O_RDONLY);
	if (fdin < 0)
	{
		printf ("*** ZVM Error open %s\n", finname);
		return -1;
	}

	char *opentag1 = "<w:t>";
	char *opentag2 = "<w:t ";
	char *closetag1 = "";
	char *closetag2 = "</w:t>";
	char *buff; // buffer for readed data from xml doc
	char *textbuff; // buffer for text, extracted from xml file

	int fsize = getfilesize_fd (fdin, NULL, 0);

	buff = (char*)  malloc (fsize + 10);
	textbuff = (char*)  malloc (fsize + 10);


	int bread = read (fdin, buff, fsize);
	//printf ("bread=%d\n", bread);
	close (fdin);
	int i;
	int ei; //счетчик извлеченный текст
	ei = 0;
	for (i = 0; i < fsize - 8; i++){
		if (strncmp(opentag1,buff + i, 5) == 0)
		{
			i = i + 5;
			while (strncmp(closetag2,buff + i, 6)) // пока не закрывается тег <\\w:t>
			{
				textbuff [ei++] = buff [i++];
			}
			textbuff[ei++] = ' ';
		}
		else if ((strncmp(opentag2, buff + i, 5) == 0))
		{
			// перематываем до начала текста
			while (buff[i++] != '>')
				;
			//i++;
			while (strncmp(closetag2,buff + i, 6))// пока не закрывается тег <\\w:t>
			{
				textbuff [ei++] = buff [i++];
			}
			textbuff[ei++] = ' ';
		}
	}
	free (buff);

	// remove &quot;&apos;
	char *q= "&quot;";
	char *ap="&apos;";
	int cq;
	for (i = 0; i < ei; i++)
	{
		if ((strncmp(q,textbuff + i,6) == 0) || (strncmp(ap,textbuff + i,6) == 0))
			for (cq = 0; cq < 6; cq++)
				textbuff[i+cq] = ' ';
	}

	int filteredbuffsize;
	filteredbuffsize = getfilteredbuffer (textbuff, ei, filteredbuff);

	printf ("*** ZVM extracted from xml (docx format) %d bytes OK!\n", filteredbuffsize);
	return filteredbuffsize;
}


/*
 * takes the text between the text docx tags, fills the buffer with the filtered text. Return size of filtered text in buffer,
 * */

int gettextfromxmlodt (char *filteredbuff)
{
	//
	int fdin; // input file descriptor
	char *finname = "content.xml"; // input file name

	fdin = open (finname, O_RDONLY);
	if (fdin < 0)
	{
		printf ("*** ZVM Error open %s\n", finname);
		return -1;
	}

	char *opentag1 = "<text:p";
	char *closetag2 = "</text:p>";
	char *buff;
	char *textbuff;

	int fsize = getfilesize_fd (fdin, NULL, 0);

	buff = (char*)  malloc (fsize + 10);
	textbuff = (char*)  malloc (fsize + 10);
	int bread = read (fdin, buff, fsize);
	close (fdin);

	int i;
	int ei; //счетчик извлеченный текст
	ei = 0;
	for (i = 0; i < fsize - 8; i++){
		if ((strncmp(opentag1, buff + i, 7) == 0))
		{
			// перематываем до начала текста
			while ((buff[i] != '>'))
				i++;
			if (buff [i-1] != '/')
			{
				while (strncmp(closetag2,buff + i, 6))// пока не закрывается тег </text:p>
				{
					textbuff [ei++] = buff [i++];
				}
				textbuff[ei++] = ' ';
			}
			else
			{
				continue;
			}
		}

	}
	free (buff);

	// remove &quot;&apos;
	char *h = "&#x0d;";
	char *q = "&quot;";
	char *a = "&apos;";

	int cq;
	for (i = 0; i < ei; i++)
	{
		if ((strncmp(q,textbuff + i,6) == 0) || (strncmp(a,textbuff + i,6) == 0) || (strncmp(h,textbuff + i,6) == 0))
			for (cq = 0; cq < 6; cq++)
				textbuff[i+cq] = ' ';
	}

	int filteredbuffsize;
	filteredbuffsize = getfilteredbuffer (textbuff, ei, filteredbuff);

	printf ("*** ZVM extracted form xml (docx format) %d bytes OK!\n", filteredbuffsize);
	free(textbuff);
	return filteredbuffsize;
}


int savefromstdin (void)
{
	FILE *f;
	f = fopen (FILENAME, "w");
	if (!f)
	{
		printf ("*** ZVM Error save file from stdin\n");
		return 1;
	}
	char c;
	int count;
	count = 0;
	while (!feof (stdin))
	{
		count++;
		c = getchar ();
		putc (c, f);
	}
	fclose (f);
	printf ("%d bytes received form stdin\n", count);
	return 0;
}

int getdoctype (char *filename)
{
	char doctypename[strlen(filename)];
	getext (filename, doctypename);
	if (strncmp (doctypename, "docx", 4) == 0)
		return 1;
	if (strncmp (doctypename, "odt", 3) == 0)
		return 2;
	if (strncmp (doctypename, "txt", 3) == 0)
		return 3;

	return 0;
}

int extractfile (char *zipfilename)
{
    //const char zipfilename[strlen(filename)];
    const char *filename_to_extract=NULL;
    const char *password=NULL;
    char filename_try[MAXFILENAME+16] = "";
    int i;
    int ret_value=0;
    int opt_do_list=0;
    int opt_do_extract=1;
    int opt_do_extract_withoutpath=1;
    int opt_overwrite=1;
    int opt_extractdir=0;
    const char *dirname=NULL;
    unzFile uf=NULL;
    //ret_value = savefromstdin ();
    //if (ret_value != 0)
    //{
    //	return ret_value;
    //}
    if (zipfilename!=NULL)
    {
        strncpy(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesnt append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

#        ifdef USEWIN32IOAPI
        fill_win32_filefunc64A(&ffunc);
        uf = unzOpen2_64(zipfilename,&ffunc);
#        else
        uf = unzOpen64(zipfilename);
#        endif
        if (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            else
            uf = unzOpen64(filename_try);
#            endif
        }
    }
    if (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        return 1;
    }
    if (opt_do_list==1)
        ret_value = do_list(uf);
    else if (opt_do_extract==1)
    {
#ifdef _WIN32
        if (opt_extractdir && _chdir(dirname))
#else
        if (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }
        if (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        else
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }
    unzClose(uf);
    if (ret_value != 0)
    {
    	printf ("Error\n");
    	return ret_value;
    }
    return ret_value;
}

int getbufffromtxt (char *filename, char *buffer)
{
	printf ("*** getbufffromtxt start extract text from txt %s\n", filename);

	long txtbuffsize;
	int fd = open (filename, O_RDONLY);
	if (fd < 0)
	{
		printf ("*** ZVM Error open %s file\n", filename);
		return -1;
	}
	txtbuffsize = getfilesize_fd(fd, NULL, 0);
	if (txtbuffsize < 0)
	{
		close (fd);
		return -1;
	}
	printf ("*** getbufffromtxt file size detected = %d\n", txtbuffsize);
	*buffer = (char *) malloc (txtbuffsize);
	//printf ("size of buffer %d\n", sizeof (buffer));
	int bread = read (fd, buffer, txtbuffsize);
	//printbuff (buffer, bread);
	if (bread < 0 || bread != txtbuffsize)
		printf ("***ZVM Error read data fron txt file %s\n", filename);
	close (fd);
	printf ("*** getbufffromtxt end extract text from txt\n");
	//printbuff (buffer, txtbuffsize);
	return txtbuffsize;
}

void printbuff (char*buff, int bufflen)
{
	printf ("***printbuff bufflen = %d\n", bufflen);
	int i;
	for (i = 0; i < bufflen; i++)
		putchar (buff[i]);
	return;
}

int main(argc,argv)
    int argc;
    char *argv[];
{


    char *filename;
    char *chname; //
    int fdout;

    char *path = "/dev/in";
    //char *path = "/home/volodymyr/git/sphinx-port/docxextract/1";
    char *prefix = "/temp";
    //char *prefix = "/home/volodymyr/temp";
    long totalbyteswrite2text, byteswrite2text;

  	DIR *dir;
	struct dirent *entry;


    byteswrite2text = 0;
    totalbyteswrite2text = 0;
//    fdout = open ("/home/volodymyr/temp/xmlpipe", O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);

    fdout = open (DEVOUTNAME, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
    if (fdout < 0 )
    {
    	printf ("*** ZVM Error open % output device\n", DEVOUTNAME);
    	return 1;
    }
	if (mkdir (prefix, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR) != 0)
	{
		printf ("*** ZVM Error create dir %s\n", prefix);
		return 1;
	}
	dir = opendir(path);
	if (dir ==0)
	{
		printf ("*** ZVM Error read dir %s, %d\n", path, dir);
		return 1;
	}
	while(entry = readdir(dir))
	{
		int temp;
		int filteredbufflen;
		filteredbufflen =0;
		if(entry->d_type != DT_DIR && (strcmp (entry->d_name, "input")) != 0)
		{
			char *filteredbuff; // buffer for filtered text extracted from file of any format
			char *buff; // temp buffer for readed data trom txt file.
			long txtbufflen;
			long filteredbufflen;

			struct filemap fmap;
			chname = malloc (strlen(path) + strlen (entry->d_name) + 2);
			sprintf (chname, "%s/%s", path, entry->d_name);
			printf ("start chname = %s\n", chname);
			fmap = getfilefromchannel(chname, prefix);
			printf ("***get from channel=%s, reafilename=%s, tempfilename=%s, real size=%ld\n", chname, fmap.realfilename, fmap.tempfilename, fmap.realfilesize);

			//const char *docxfilename = "document.xml";
			//const char *odtfilename = "content.xml";

			if (fmap.realfilesize <= 0)
			{
				free (chname);
				continue;
			}
			filteredbuff = (char *) malloc (fmap.realfilesize * 3);
			dt = getdoctype (fmap.realfilename);
			printf ("doc ty	pe %d\n", dt);
			int retextractcode; //
			long fsize;
			switch (dt) {
			case 1://docx
				retextractcode = extractfile (fmap.tempfilename); // unzip incoming file place xml contents on file document.xml
				if (retextractcode != 0)
					continue;
				filteredbufflen = gettextfromxml (filteredbuff); // extracting and filtering text data from content.xml file
		    	break;
			case 2://odt
				retextractcode = extractfile (fmap.tempfilename); // unzip incoming file
				if (retextractcode != 0)
					continue;
		    	filteredbufflen = gettextfromxmlodt (filteredbuff);
		    	break;
			case 3://txt
				buff = malloc (fmap.realfilesize + 2);
				txtbufflen = getbufffromtxt (fmap.tempfilename , buff);
				filteredbufflen = getfilteredbuffer (buff, txtbufflen, filteredbuff);
		    	break;
			}

			//printbuff(filteredbuff, filteredbufflen);
			printf ("filteredbufflen = %d\n", filteredbufflen);
			int tempwritebytes2channel;

			if (filteredbufflen > 0)
			{
//				printf ("*****filteredbufflen %d\n", filteredbufflen);
//				for (temp = 0; temp < filteredbufflen; temp++)
//					putchar (filteredbuff[temp]);
				printf ( "start send data to xmlpipecreator \n");
				tempwritebytes2channel = puttext2channel (filteredbuff, filteredbufflen, fmap.realfilename, fdout);
				printf ( "end send data to xmlpipecreator tempwritebytes2channel - %d\n", tempwritebytes2channel);
			}
			else
			{
				//free (buff);
				//buff =  0;
				//free(filteredbuff);
				//filteredbuff = 0;
				continue;
			}
			//free (buff);
			//buff = 0;
			//free(filteredbuff);
			//filteredbuff =0;
			printf ("end chname = %s\n", chname);
		//	free (chname);
		}
	}
	close (fdout);
	printf ("all ok\n");
	return 0;

}