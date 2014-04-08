#!/bin/bash
	#1508

if [ $# -ne 1 ]
then
  echo "Usage: `basename $0` search_path"
  exit $WRONG_ARGS
else
directory=$1
fi

#List of statically assigned nodes id that belongs to test cluster.
#2 - indexer; 1-xmlpipecreator; 3 - pdf extractor; 4 - docx, txt, odt extractors; 5 doc extractor
#  11 - node ID for first fiesender in cluster
FILECOUNT=11
SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

#removing old manifest and nvram files
rm -f manifest/*.manifest nvram/*.nvram

#Generate manifests for:
#doc text extractor.
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/doc.manifest.template > manifest/doc.manifest
#pdf text extractor
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/pdf.manifest.template > manifest/pdf.manifest
#docx, txt, odt text extractor 
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/txt.manifest.template > manifest/txt.manifest
#node for other file types or if the file size is too big
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/other.manifest.template > manifest/other.manifest

#Generate manifests for:
#xmlpipecreator
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/xmlpipecreator.manifest.template > manifest/xmlpipecreator.manifest
#indexer
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/indexer.manifest.template > manifest/indexer.manifest
#search
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/search.manifest.template > manifest/search.manifest

#Generate nvram files for indexer, search, xmlpipecreator, extractors
cp nvram.template/txt.nvram.template 		nvram/txt.nvram
cp nvram.template/pdf.nvram.template 		nvram/pdf.nvram
cp nvram.template/doc.nvram.template 		nvram/doc.nvram
cp nvram.template/other.nvram.template 		nvram/other.nvram
cp nvram.template/xml.nvram.template 		nvram/xml.nvram
cp nvram.template/search.nvram.template 	nvram/search.nvram
cp nvram.template/indexer.nvram.template 	nvram/indexer.nvram

DEVICENAME=/dev/in/filesender

scan() {
  local x;
  for file in "$1"/*; do
#    filename=${e##*\/};
    if [ -d "$file" -a ! -L "$file" ]
    then
      scan "$file";
    else
	if [ -f "$file" ]
	then
		file="$file"
		filename="${file%.}"
		fileext="${file##*.}"
		TEMP=$(stat -c %s "$file")
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,	"$DEVICENAME-$FILECOUNT", 0, 0, 99999999, 99999999, 0, 0"
		./filesendermanifestgenerator.sh $FILECOUNT "$filename"
		./filesendernvramgenerator.sh $FILECOUNT "$filename" 
		echo $CHANNELNAME >> manifest/txt.manifest
		echo $CHANNELNAME >> manifest/doc.manifest
		echo $CHANNELNAME >> manifest/pdf.manifest
		echo $CHANNELNAME >> manifest/other.manifest
		let FILECOUNT=FILECOUNT+1
	fi
    fi
  done
}

if [ -d "$directory" ]
then
  scan "$directory";
fi

#extrace files from directory which can contain spaces in name
#for file in "$directory"/*
#do


