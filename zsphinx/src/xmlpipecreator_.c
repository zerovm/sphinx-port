#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "zvmfileutil.h"
#include "../../config/config.h"
#include "zlib.h"
#include "xmlpipecreator_.h"


#define myout outf
#define fout(st) fprintf (myout, "%s", st);
#define foutc(st) fprintf (myout, "%c", st);
#define USE_XMLPIPE2 0
#define MAXID_DEV_NAME_IN "/dev/input" 		// device for load document max ID
#define MAXID_DEV_NAME_OUT "/dev/output" 	// device for save document max ID
#define DEV_OUTPUT_NAME "/dev/out/indexer"


const char *xml_open = "\
<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";

const char *xml_close = "\
</sphinx:docset>\n";


const char *open_schema_el = "\
<sphinx:docset>\n\
<sphinx:schema>\n";

const char *close_shema_el = "\
</sphinx:schema>\n";


const char *static_fields = "\
<sphinx:attr name=\"static_sphinx_meta\" type=\"json\"/> \n\
<sphinx:field name=\"static_content\"/> \n\
<sphinx:field name=\"static_metatags\" type=\"string\" />\n";



#define pritnf printf // дурацкий макрос

/*
#define ZVMDEBUG
#undef ZVMDEBUG
*/



FILE *outf;

int docID=1;

//MAX size of XML Headrer buffer
int iXML_Head_Size = 1024;


void mylistdir_xmlpipe (int fd, char *path, struct field_list fl)
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
		if(!(entry->d_type == DT_DIR && (strcmp (entry->d_name, "input")) != 0))
		{
			char devname [strlen (path) + strlen (entry->d_name) + 5];
			sprintf (devname, "%s/%s", path, entry->d_name);
			LOG_ZVM ("***ZVMLog", "channel name", "s", devname, 1);
			if (strncmp (devname, CHECK_INDEXER_XML_DEV_NAME, strlen (CHECK_INDEXER_XML_DEV_NAME)) == 0)
				continue;
			int doccount;
			doccount = getdatafromchannel (fd, devname, docID, fl);
			LOG_ZVM ("***ZVMLog", "doc in current channel", "d", doccount - docID, 1);
			docID = doccount;
		}
	}
	LOG_ZVM ("***ZVMLog", "total docs", "d", docID, 1);
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

char *check_Buff_Size (char *buff, int *maxSize, int iAddSize)
{
	if (*maxSize <= (strlen (buff) + iAddSize))
	{
		char * old_buff = buff;

		buff = (char *) malloc (sizeof (char) * (*maxSize) * 2);
		strncpy (buff, old_buff, *maxSize);
		*maxSize *= 2;
		free (old_buff);
	}
	return buff;
}

char *create_Blank_XML_Head_Open ()
{
	int i = 0;
	char *XML_Head = (char *) malloc (sizeof (char) * iXML_Head_Size);

	sprintf (XML_Head, "%s%s", xml_open, open_schema_el);

	// attrs
	for (i =  0; i < blank_field_count; i++)
	{
		XML_Head = check_Buff_Size (XML_Head, &iXML_Head_Size, (strlen (blank_field_list[i]) + strlen (blank_field_list_types[i]) + 100));
		sprintf (XML_Head + strlen (XML_Head), "<sphinx:field name=\"%s\" type=\"%s\" />\n",blank_field_list[i], blank_field_list_types[i]);
	}

	// indexed fields
	for (i =  0; i < blank_attr_count; i++)
	{
		XML_Head = check_Buff_Size (XML_Head, &iXML_Head_Size, (strlen (blank_attr_list[i]) + strlen (blank_attr_list_types[i]) + 100));
		sprintf (XML_Head + strlen (XML_Head), "<sphinx:attr name=\"%s\" type=\"%s\" />\n",blank_attr_list[i], blank_attr_list_types[i]);
	}
	return XML_Head;
}

char *create_Custom_XML_Head (struct field_list fl)
{
	char *XML_Head = NULL;
	int i = 0, j = 0;

	XML_Head = (char *) create_Blank_XML_Head_Open ();

	if (fl.fieldCount >= 0)
	{
		for (i = 0; i < fl.fieldCount; i++)
		{
			int static_attr = 0;
			for ( j = 0; j < blank_attr_count; j++)
			{
				char *buff1 = NULL;
				char *buff2 = NULL;
				buff1 = my_strtolower( blank_attr_list[j]);
				buff2 = my_strtolower(fl.fields[i]);
				if (strncmp (buff1, buff2, MY_MAX(buff1,buff2) ) == 0 )
					static_attr = 1;
				free (buff1);
				free (buff2);
			}
			for ( j = 0; j < blank_field_count; j++)
			{
				char *buff1 = NULL;
				char *buff2 = NULL;
				buff1 = my_strtolower(blank_field_list[j]);
				buff2 = my_strtolower(fl.fields[i]);
				if (strncmp (buff1,buff2, MY_MAX(buff1, buff2)) == 0 )
					static_attr = 1;
			}
			if (static_attr == 0)
			{
				XML_Head = check_Buff_Size (XML_Head, &iXML_Head_Size, 100);
				sprintf(XML_Head + strlen(XML_Head), "<sphinx:attr name=\"%s\" type=\"%s\" />\n", fl.fields[i],fl.types[i]);
			}
		}
	}
	//close head (schema)
	XML_Head = check_Buff_Size (XML_Head, &iXML_Head_Size, 21);
	sprintf (XML_Head + strlen(XML_Head), "</sphinx:schema>\n\n");
	return XML_Head;
}

// заголовок XML потока
void createxmlpipe (int fd, struct field_list fl)
{
	char *XML_Head = create_Custom_XML_Head (fl);
	int bwrite;
	bwrite = write (fd, XML_Head, strlen (XML_Head));
	return;
}

// footer of sphinx document in XML stream
void closexmlpipe (int fd)
{
	int bwrite;
	bwrite = write (fd, xml_close, strlen (xml_close));
}	

int checkIndexer ()
{
	int fd = 0;
	int BUFFSIZE = 3;
	char buff[BUFFSIZE];
	int readbytes = 0;
	fd = open( CHECK_INDEXER_XML_DEV_NAME, O_RDONLY);
	if (fd <= 0 )
	{
		printf ("*** Error open indexer check device\n");
		return 0;
	}

	if ( (readbytes = read (fd, buff, BUFFSIZE - 1)) != (BUFFSIZE - 1))
	{
		printf ("*** Error read %d bytes form indexer check device\n", BUFFSIZE - 1);
		close (fd);
		return 0;
	}

	if (strncmp(buff, "OK",2) == 0)
	{
		LOG_ZVM ("***ZVMLog", "indexer config", "s", "OK", 1);
		close (fd);
		return 1;
	}
	else
	{
		close (fd);
		return 0;
	}
}

char * check_Field_type (char *test)
{
	char *types[] = {
			"int",
			"string",
			"bigint",
			"timestamp",
			"float",
			"json"
	};

	int types_count = sizeof (types) / sizeof (char *);
	int i = 0;
	int type_index = -1;
	int test_OK = 0;
	for ( i = 0; i < types_count; i++)
	{
		char *buff1 = NULL;
		char *buff2 = NULL;
		buff1 = my_strtolower(types[i]);
		buff2 = my_strtolower(test);

		if (strncmp (buff1,buff2, MY_MAX(buff1, buff2)) == 0 )
			type_index = i;
	}

	if (type_index >= 0 && type_index < types_count)
		return types[type_index];
	if (strncmp (my_strtolower(test) ,my_strtolower("str"), MY_MAX(my_strtolower(test), my_strtolower("str"))) == 0 )
		return "string";
	if (strncmp (my_strtolower(test) ,my_strtolower("integer"), MY_MAX(my_strtolower(test), my_strtolower("integer"))) == 0 )
		return "int";
	if (strncmp (my_strtolower(test) ,my_strtolower("time"), MY_MAX(my_strtolower(test), my_strtolower("time"))) == 0 )
		return "timestamp";
	if (strncmp (my_strtolower(test) ,my_strtolower("biginteger"), MY_MAX(my_strtolower(test), my_strtolower("biginteger"))) == 0 )
		return "bigint";

	return NULL;
}


//////////////////////////////////////////////////////////////////////////////////
//	return 1 if new field name is unique
//	return 0 if new field name already exists in field list
//////////////////////////////////////////////////////////////////////////////////
int check_Unique (struct field_list fl, char *new_field_name)
{
	int i = 0;

	// check blank attr
	for (i = 0 ; i < blank_attr_count; i++)
		if (MY_STRNCMP (blank_attr_list[i], new_field_name) == 0 )
		{
			return 0;
		}
	//check blank field
	for (i = 0 ; i < blank_attr_count; i++)
		if (MY_STRNCMP (blank_field_list[i], new_field_name) == 0 )
		{
			return 0;
		}

	// check dynamic attr
	if (fl.fieldCount <= 0 )
		return 1; //
	for ( i = 0; i < fl.fieldCount; i++)
	{

		if (MY_STRNCMP (fl.fields[i], new_field_name) == 0 )
		{
			return 0;
		}
	}
	return 1;
}

struct field_list getList (char *deviceName)
{
	struct field_list fl;
	char buff[MAX_FIELD_NAME_LENGTH];
	char buff_n [MAX_FIELD_NAME_LENGTH]; // buff for field name
	char buff_t [MAX_FIELD_NAME_LENGTH]; // buff for field type
	int fieldCount =0, i =0;
	FILE *f;
	if (deviceName == NULL)
	{
		printf ("***ZVM Error. Field list load, wrong device name");
		fl.fieldCount = 0;
		fl.fields = NULL;
		return fl;
	}
	f = fopen (deviceName, "r");
	if (f == NULL)
	{
		fl.fieldCount = 0;
		fl.fields = NULL;
		return fl;
	}
	while ( !feof (f) )
    {
		if ( fgets (buff , MAX_FIELD_NAME_LENGTH , f) != NULL )
			if (strlen (buff) > 1)
				if (sscanf (buff, "%s %s", buff_n, buff_t) == 2)
					fieldCount++;
    }
	fl.fields = (char ** ) malloc (sizeof (char*) * fieldCount);
	fl.types = (char ** ) malloc (sizeof (char*) * fieldCount);
	fseek (f, 0, SEEK_SET);
	i = 0;
	while ( !feof (f) )
    {
		if ( fgets (buff , MAX_FIELD_NAME_LENGTH , f) != NULL )
		{
			if (strlen (buff) > 1)
			{
				if (sscanf (buff, "%s %s", buff_n, buff_t) != 2)
					continue;

				if ( check_Unique (fl, buff_n) == 0 )
					continue;

				if ( (fl.types[i] = check_Field_type (buff_t)) == NULL)
					continue;

				fl.fields[i] = (char *) malloc (sizeof (char) * strlen (buff_n));
				strncpy (fl.fields[i],buff_n, strlen (buff_n)+1);
				LOG_ZVM ("***ZVMLog", "XML attr field name", "s", fl.fields[i], 2);
				LOG_ZVM ("***ZVMLog", "XML attr field type", "s", fl.types[i], 2);
				i++;
				fl.fieldCount = i;
			}
		}
    }
	fl.fieldCount = i;
	fclose (f);
	return fl;
}

void SendDelete (int fd, struct field_list fl)
{
	unsigned long crc = crc32(0L, Z_NULL, 0);

	char realfilename[MAXFIENAME];

	int bwrite = 0;
	//
	while ( fgets (realfilename, MAXFIENAME, stdin) != NULL )
	{
		realfilename [strlen (realfilename) - 1] = '\0';
		crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, (const Bytef*) realfilename, strlen (realfilename));
		if (crc == 0)
			continue;
		printdochead (fd, crc);
		int i = 0;

		for (i = 0; i < blank_field_count; i++)
			bwrite = write_XML_Elemet (fd, blank_field_list[i], "");
		for (i = 0; i < blank_attr_count; i++)
			bwrite = write_XML_Elemet (fd, blank_attr_list[i], "");
		for (i = 0; i < fl.fieldCount; i++)
			bwrite = write_XML_Elemet (fd, fl.fields[i], "");

		printdocfooter(fd);
	}
	return;
}


int xml_main(int argc, char **argv)
{
	char *serversoft = getenv ("SERVER_SOFTWARE");
	LOG_SERVER_SOFT;
	LOG_NODE_NAME;

	char c = '0';
	char p[] = "/dev/in";
	int fd;

	int iOptindexer = 1;
	int bIndexerReady = 0;
	if (Mode != single_operation)
		bIndexerReady = checkIndexer();
	else
		bIndexerReady = 1;

	int bDeleteFromIndex = 0;
	int bTemplateIndex = 0;
	int bSaveXML = 0;
	int i = 0;
	char deviceoutputname[250];
	sprintf (deviceoutputname, "%s", DEV_OUTPUT_NAME);
	for ( i = 0; i < argc; i++)
	{
		if ((strncmp(argv[i], "--savexml", strlen (argv[i]))) == 0)
			bSaveXML = 1;
		if ((strncmp(argv[i], "--delete", strlen (argv[i]))) == 0)
			bDeleteFromIndex = 1;

		if ((strncmp(argv[i], "--template", strlen (argv[i]))) == 0)
			bTemplateIndex = 1;

	}

	if (bSaveXML == 1)
		sprintf (deviceoutputname, "/dev/output");
	else
		if (bIndexerReady != 1)
		{
			printf ("***Error indexer not ready\n");
			return 1;
		}


	if (Mode == single_operation)
    	sprintf (deviceoutputname, "%s", XML_SINGLE_MODE_OUTPUT_FILE);
	else
		if(bDeleteFromIndex == 0)
			LOG_ZVM ("***ZVMLog", "incoming channels dir", "s", p, 1);

	LOG_ZVM ("***ZVMLog", "output channel name", "s", deviceoutputname, 1);


	fd = open (deviceoutputname, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IWOTH | S_IRUSR | S_IWUSR);
	if (fd <= 0)
	{
		printf ("*** ZVM. Error open %s deivce\n", DEV_OUTPUT_NAME);
		return 1;
	}


	struct field_list fl = getList ("/dev/input");

	LOG_ZVM ("***ZVMLog", "doc count", "d", docID, 1);
	createxmlpipe (fd, fl);
	if (bDeleteFromIndex == 0 && bTemplateIndex == 0)
	{
		LOG_ZVM ("***ZVMLog", "incoming channels dir", "s", deviceoutputname, 1);
		int doccount = 0;
		if (Mode == single_operation)
			doccount = getdatafromchannel (fd, EXTRACTOR_SINGLE_MODE_OUTPUT_FILE, docID, fl);
		else
			mylistdir_xmlpipe (fd, p, fl);
	}
	else if ((bDeleteFromIndex == 1 && bTemplateIndex == 0))
	{
		LOG_ZVM ("***ZVMLog", "incoming channels dir", "s", p, 1);
		SendDelete (fd, fl);
	}
	else if ((bDeleteFromIndex == 0 && bTemplateIndex == 1))
	{
/*
		LOG_ZVM ("***ZVMLog", "incoming channels dir", "s", p, 1);
		SendDelete (fd);
*/
	}
	closexmlpipe (fd);
	close (fd);
	printf ("*** ZVM xmlpipe - OK\n");
	return 0;
}
