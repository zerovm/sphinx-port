#!/bin/bash


if [ $# -ne 1 ]
then
  echo "Usage: `basename $0` search_path"
  exit $WRONG_ARGS
else
directory=$1
fi

echo remove old nexes, outs, manifests and nvrams configuration
rm -f *.nexe
rm -f manifest/*.manifest
rm -f *data/out/*
rm -f nvram/*.nvram


echo generate zsphinx.conf from template
./rwgenerator.sh
echo copy nexe
cp ../src/xmlpipecreator 	xmlpipecreator.nexe
cp ../src/text 	text.nexe
cp ../src/indexer 		indexer.nexe
cp ../src/search 		search.nexe
cp ../zxpdf-3.03/xpdf/pdftotext pdftotext.nexe
cp ../docxextract/docxtotext docx.nexe
cp ../docxextract/docxtotext odt.nexe
cp ../antiword-0.37/antiword doc.nexe

echo generate manifest
#генерируем манифесты для всех нодов а также для search
./allmanifestgenerator.sh "$1"

#считаем количество всех нодов
NODECOUNT=0
for file in manifest/*.manifest
do
	let NODECOUNT=NODECOUNT+1
done

let NODECOUNT=NODECOUNT-1

./ns_start.sh $NODECOUNT


#1 - indexer, 2-xmlpipecreator, 3, search, 4 - нод первого документа, 5 - и т.д.  
TEMPNODECOUNT=0
for file in manifest/*.manifest
do
	#if [ $file != manifest/xmlpipecreator.manifest ] && [ $file != manifest/indexer.manifest ] && 
	if [ $file != manifest/search.manifest ] && [ $file != manifest/indexer_merge.manifest ] && [ $file != manifest/indexer_delta.manifest ]
	then
		let TEMPNODECOUNT=TEMPNODECOUNT+1
		echo nodecount $NODECOUNT
		echo tempnodecount $TEMPNODECOUNT
		if [ $NODECOUNT -eq $TEMPNODECOUNT ]
		then
			echo nodecount $NODECOUNT tempnodecount $TEMPNODECOUNT 
			echo run $file
			${ZVM_PREFIX}/zerovm -M$file
			echo OK
		else
			echo nodecount $NODECOUNT tempnodecount $TEMPNODECOUNT q
			echo run $file
			${ZVM_PREFIX}/zerovm -M$file &
			echo OK
		fi
	fi
done

#${ZVM_PREFIX}/zerovm -Mmanifest/xmlpipecreator.manifest &
#echo run manifest/xmlpipecreator.manifest
#echo run manifest/indexer.manifest
#${ZVM_PREFIX}/zerovm -Mmanifest/indexer.manifest

./ns_stop.sh

sleep 1

cat data/out/indexer_stdout.data

${ZVM_PREFIX}/zerovm -Mmanifest/search.manifest 
cat data/out/search_stdout.data
