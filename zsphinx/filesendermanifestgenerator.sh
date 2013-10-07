#!/bin/bash
#{DOCNUMBER} порядковый номер документа
#{FILE_NAME} имя подключаемого файла

if [ $# -lt 2 ]
then
	echo "Usage: `basename $0` {DOCNUMBER} {FILE_NAME}"
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

sed s@{ABS_PATH}@$ABS_PATH@ manifest.template/filesender.manifest.template | \
sed s@{DOCNUMBER}@$DOCNUMBER@ | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > manifest/filesender$DOCNUMBER.manifest
