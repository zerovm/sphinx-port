#!/bin/bash
#{DOCNUMBER} порядковый номер документа
#{FILE_NAME} имя подключаемого файла

if [ $# -ne 2 ]
then
	echo "Usage: `basename $0` {DOCNUMBER} {FILE_NAME}"
#	echo "test" $DOCNUMBER $FILE_NAME $TYPENUMBER 
 	exit $WRONG_ARGS
else
	DOCNUMBER=$1
	FILE_NAME=$2
fi

TEMP=$(stat -c %s $FILE_NAME)

#echo nvram generator 
#echo docname $DOCNUMBER
#echo 'file name' $FILE_NAME

sed s@{DOCNUMBER}@$DOCNUMBER@ nvram.template/filesender.nvram.template | \
sed s@{CONTETNLENGTH}@"$TEMP"@g | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > nvram/filesender$DOCNUMBER.nvram

#sed s@{TYPENUMBER}@$TYPENUMBER@ nvram.template/$FILETYPE.nvram.template | \
#sed s@{FILE_NAME}@"$FILE_NAME"@g > nvram/$FILETYPE$DOCNUMBER.nvram
