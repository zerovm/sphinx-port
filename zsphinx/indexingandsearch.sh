#!/bin/bash


if [ $# -ne 1 ] && [ $# -ne 2 ]
then
  echo "Usage: `basename $0` search_path"
  exit $WRONG_ARGS
fi
if [ $# -eq 1 ]
then
directory=$1
query="test"
fi
if [ $# -eq 2 ]
then
directory=$1
query=$2
fi

echo $directory
echo $query

echo $query > data/search_stdin.data

echo remove old nexes, outs, manifests and nvrams configuration
rm -f *.nexe
rm -f manifest/*.manifest
rm -f data/out/*
rm -f nvram/*.nvram

#echo generate zsphinx.conf from template
./rwgenerator.sh
echo copy nexes
cp ../src/xmlpipecreator 	xmlpipecreator.nexe
#cp ../src/text 			txt.nexe
cp ../src/indexer 		indexer.nexe
cp ../src/filesender 		filesender.nexe
cp ../src/search 		search.nexe
cp ../zxpdf-3.03/xpdf/pdftotext pdf.nexe
cp ../docxextract/docxtotext txt.nexe
#cp ../docxextract/docxtotext odt.nexe
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
	if [ $file != manifest/search.manifest ] && [ $file != manifest/indexer_merge.manifest ] && [ $file != manifest/indexer_delta.manifest ]&& [ $file != manifest/indexer.manifest ]
	then
#		let TEMPNODECOUNT=TEMPNODECOUNT+1
#		echo nodecount $NODECOUNT
#		echo tempnodecount $TEMPNODECOUNT
#		if [ $NODECOUNT -eq $TEMPNODECOUNT ]
#		if [ "$file" = "m/" ] 
#		then
#			echo nodecount $NODECOUNT tempnodecount $TEMPNODECOUNT 
			echo run $file
			${ZVM_PREFIX}/zerovm -P -M$file &
#			echo OK
#		else
#			echo nodecount $NODECOUNT tempnodecount $TEMPNODECOUNT q
#			echo run $file
#			${ZVM_PREFIX}/zerovm -M$file &
#			echo OK
#		fi
	fi
done
echo run manifest/indexer.manifest
time ${ZVM_PREFIX}/zerovm -P -Mmanifest/indexer.manifest
#${ZVM_PREFIX}/zerovm -Mmanifest/xmlpipecreator.manifest &
#echo run manifest/xmlpipecreator.manifest
#echo run manifest/indexer.manifest
#${ZVM_PREFIX}/zerovm -Mmanifest/indexer.manifest

./ns_stop.sh


sleep 2

cat data/out/indexer_stdout.data

./mergedelta+main.sh

./search.sh "channel"
