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

sed s@{TYPENUMBER}@$TYPENUMBER@ nvram.template/$FILETYPE.nvram.template | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > nvram/$FILETYPE$DOCNUMBER.nvram

