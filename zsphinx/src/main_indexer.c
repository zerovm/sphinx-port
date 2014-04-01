/*
 * main_indexer.c
 *
 *  Created on: Dec 11, 2013
 *      Author: Volodymyr Varchuk
 */
#include "../../src/indexer_.h"
#include "../../zxpdf-3.03/xpdf/pdftotext_.h"
#include "../../docxextract/docxtotext_.h"
#include "../../antiword-0.37/main_u_.h"

#include "other_.h"
#include "filesender_.h"
#include "xmlpipecreator_.h"
#include "zvmfileutil.h"
#include "opt.h"

#include <stdio.h>
#include <sys/stat.h>


#define SIZEOF(A) sizeof(A)/sizeof(char*)

void printdata (char *filename)
{

	//"filesender.dat";
	printf ("\n\n print %s \n\n", filename);
	char buff[1024];
	int c;
	FILE* f;
	f = fopen (filename, "r");
	if (f == NULL)
		return;
	while((fgets(buff,1024,f)) != NULL)
		printf("%s",buff);
	printf (" \n\n end print %s \n\n", filename);
	fclose (f);
	return;
}

int checkindexfiles (char * indexdir, char * indexname)
{
	int filenamesize = 0;
	if (indexdir != NULL && indexname != NULL)
		filenamesize = strlen(indexdir)+ strlen (indexname) + 8;
	else
		return 1;
	char indexfilename [filenamesize];

	int i = 0;
	sprintf(indexfilename, "/%s/%s.sph", indexdir, indexname );

	FILE *f;
	f = fopen (indexfilename, "r" );
	if (!f)
		return 1;
	fclose (f);

	return 0;
}


int Add_To_Index ()
{
	fileTypeInfo_t fti;

	newbufferedunpack( I_DEVINPUTDATA );

	int indexfileOK = checkindexfiles ("index", "mainindex");
	printf ("mainindex - %s\n", indexfileOK == 1 ? "Error" : "OK");

	indexfileOK = checkindexfiles ("index", "mainindex");
	printf ("deltaindex - %s\n", indexfileOK == 1 ? "Error" : "OK");

	int retcode = 0;

	fti = checkMAxFileSize ();

	char *fs_argv[] = {"filesender"};
	int fs_argc = SIZEOF(fs_argv);

	int fs_ret = filesender_main(fs_argc, fs_argv);

	//printdata (FILESENDER_SINGLE_MODE_OUTPUT_FILE);

	int extr_ret = 0;

	if (fti.iExtractorType == txt )
	{
		char *docx_argv[] = {"txt"};
		int docx_argc = SIZEOF(docx_argv);
		extr_ret = retcode = docx_main(docx_argc, docx_argv);
	}
	if (fti.iExtractorType == doc )
	{
		char *doc_argv[] = {"doc", "temp.doc"};
		int doc_argc = SIZEOF(doc_argv);
		extr_ret = doc_main(doc_argc, doc_argv);
	}
	if (fti.iExtractorType == pdf )
	{
		char *pdf_argv[] = {"pdf"};
		int pdf_argc = SIZEOF(pdf_argv);
		extr_ret = pdf_main(pdf_argc, pdf_argv);
	}
	if (fti.iExtractorType == other )
	{
		char *other_argv[] = {"other"};
		int other_argc = SIZEOF (other_argv);
		extr_ret = other_main(other_argc, other_argv);
	}

//	printdata (EXTRACTOR_SINGLE_MODE_OUTPUT_FILE);

	char *xml_argv [] = {"xmlpipecreator"};
	int xml_argc = SIZEOF(xml_argv);
	int ret_xml = xml_main(xml_argc, xml_argv);

	printdata (XML_SINGLE_MODE_OUTPUT_FILE);

	char *ind_argv[] = {"indexer", "--config", "index/zsphinx.conf", "deltaindex"};
	int ind_argc = SIZEOF(ind_argv);

	int ret_ind = indexer_main(ind_argc, ind_argv);

	char *merge_argv[] = {"indexer", "--config", "index/zsphinx.conf", "--merge", "mainindex", "deltaindex", "--packindex"};
	int merge_argc = SIZEOF(merge_argv);

	int ret_merge = indexer_main(merge_argc, merge_argv);

	if (ret_merge == 0 )
	{
		newbufferedpack((char *)I_DEVOUTPUTDATA, (char *) INDEXDIRNAME);
	}

	return 0;
}

int Delete_From_Index ()
{

	return 0;
}

int main (int argc, char * argv[])
{
	mylistdir("/");

	int do_Act = 0;
	int ret = 0;
	int i = 0;

	Mode = single_operation;

	do_Act = get_Act(argc, argv);

	switch (do_Act) {
	case 1:
		Add_To_Index ();
		break;
	case 2:
		Delete_From_Index();
		break;
	}

	return ret;
}
