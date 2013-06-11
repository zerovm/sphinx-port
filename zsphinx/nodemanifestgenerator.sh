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

SCRIPT=$(readlink -f "$0")
ABS_PATH=`dirname "$SCRIPT"`/
#echo $ABS_PATH 

FILETYPE="${FILE_NAME##*.}"
echo 

sed s@{ABS_PATH}@$ABS_PATH@ manifest.template/$FILETYPE.manifest.template | \
sed s@{DOCNUMBER}@$DOCNUMBER@ | \
sed s@{TYPENUMBER}@$TYPENUMBER@ | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > manifest/$FILETYPE$DOCNUMBER.manifest

