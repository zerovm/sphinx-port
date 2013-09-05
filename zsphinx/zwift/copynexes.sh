#!/bin/bash

echo -e "== \t" removing old nexes
rm -f /search/sys/*.nexe

echo -e "== \t" copying nexes
cp ../../src/xmlpipecreator 		search/sys/xmlpipecreator.nexe
cp ../../src/indexer 			search/sys/indexer.nexe
cp ../../src/filesender 		search/sys/filesender.nexe
cp ../../src/search 			search/sys/search.nexe
cp ../../zxpdf-3.03/xpdf/pdftotext 	search/sys/pdf.nexe
cp ../../docxextract/docxtotext 	search/sys/txt.nexe
cp ../../antiword-0.37/antiword 	search/sys/doc.nexe

