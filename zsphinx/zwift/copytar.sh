#!/bin/bash


echo -e "== \t" removing old tars 
rm -f /search/sys/*.tar
	
echo -e "== \t" copying archives	
cp ../data/antiword.tar 	search/sys/antiword.tar
cp ../data/confpdf.tar 		search/sys/confpdf.tar

