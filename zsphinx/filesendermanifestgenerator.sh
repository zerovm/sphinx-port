#!/bin/bash
#{DOCNUMBER} порядковый номер документа
#{FILE_NAME} имя подключаемого файла

if [ $# -ne 3 ]
then
	echo "Usage: `basename $0` {DOCNUMBER} {FILE_NAME} {TYPENUMBER}"
	echo "test" $DOCNUMBER $FILE_NAME $TYPENUMBER 
 	exit $WRONG_ARGS
else
	DOCNUMBER=$1
	FILE_NAME=$2
	TYPENUMBER=$3
fi

echo "Filesender manifest is Node #$DOCNUMBER for indexing file: $FILE_NAME"

SCRIPT=$(readlink -f "$0")
ABS_PATH=`dirname "$SCRIPT"`/
#echo $ABS_PATH 

FILETYPE="${FILE_NAME##*.}"

if [ "$FILETYPE" = "txt" ] || [ "$FILETYPE" = "docx" ] || [ "$FILETYPE" = "odt" ]
then 
	DOC_TYPE_NUMBER=4
	DOC_TYPE="txt"
fi

if [ "$FILETYPE" = "pdf" ]
then
	DOC_TYPE_NUMBER=3
	DOC_TYPE="pdf"
fi

if [ "$FILETYPE" = "doc" ] 
then
	DOC_TYPE_NUMBER=5
	DOC_TYPE="doc"
fi

sed s@{ABS_PATH}@$ABS_PATH@ manifest.template/filesender.manifest.template | \
sed s@{DOCNUMBER}@$DOCNUMBER@ | \
sed s@{DOC_TYPE_NUMBER}@$DOC_TYPE_NUMBER@ | \
sed s@{DOC_TYPE}@$DOC_TYPE@ | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > manifest/filesender$DOCNUMBER.manifest
