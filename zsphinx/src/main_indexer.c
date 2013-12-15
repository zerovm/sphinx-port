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
#include "zvmfileutil.h"

#include <stdio.h>

int main (int argc, char * argv[])
{
	int ret = 0;

	ret = doc_main(argc, argv);
	printf ("doc %d\n", ret);
	ret = docx_main(argc, argv);
	printf ("docx %d\n", ret);
	ret = indexer_main(argc, argv);
	printf ("indexer %d\n", ret);
	ret = pdf_main(argc, argv);
	printf ("pdf %d\n", ret);
	return ret;
}
