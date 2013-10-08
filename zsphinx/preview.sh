#!/bin/bash

#if [ $# -ne 1 ]
#	echo "Usage: `basename $0` searched words"
 #	exit $WRONG_ARGS
#then
	REQUEST=$1
#fi

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

echo "search result:" > result_preview.txt

./search.sh "-ws "$REQUEST""

FILENAME=data/out/search_stdout.data

DOCCOUNT=1

cat $FILENAME | while read LINE; do 

	# extract file name
	INDEX=`expr index "$LINE" ";"`
	let INDEX=INDEX-1
	FILENAME=`expr substr "$LINE" 1 $INDEX`

	#extract start position
	STRLEN=${#LINE}
	let INDEX=INDEX+9
	let STRLEN=STRLEN-INDEX	
	LASTSTR=`expr substr "$LINE" $INDEX $STRLEN`
	INDEXSTART=`expr index "$LASTSTR" ";"`
	let INDEXSTART=INDEXSTART-1
	START=`expr substr "$LASTSTR" 1 $INDEXSTART`

	#extract end position
	let INDEXSTART=INDEXSTART+7
	END=`expr substr "$LASTSTR" $INDEXSTART $STRLEN`
	if [ ${#FILENAME} -ne 0 ]
	then
		CMD_LINE="--search $START $END"
		fileext="${FILENAME##*.}"
		if [ "$fileext" = "txt" ] || [ "$fileext" = "docx" ] || [ "$fileext" = "odt" ];
		then
			fileext="txt"
		elif [ "$fileext" != "pdf" ] && [ "$fileext" != "doc" ];
		then
			fileext="other"
		fi
		
		sed s@{CMD_LINE}@"$CMD_LINE"@g nvram.template/"$fileext"-snippet.nvram.template > nvram/"$fileext"-"$DOCCOUNT"-snippet.nvram
		ENV="name=filename, value=$FILENAME"
		ENV2="name=PATH_INFO, value=$FILENAME"
		echo "$ENV" >> nvram/"$fileext"-"$DOCCOUNT"-snippet.nvram
		echo "$ENV2" >> nvram/"$fileext"-"$DOCCOUNT"-snippet.nvram

		ABS_PATH=`dirname "$SCRIPT"`/
		sed s@{ABS_PATH}@$ABS_PATH@ manifest.template/$fileext-snippet.manifest.template | \
		sed s@{DOCCOUNT}@"$DOCCOUNT"@ | \
		sed s@{FILE_NAME}@"$FILENAME"@g > manifest/"$fileext"-"$DOCCOUNT"-snippet.manifest
		${ZVM_PREFIX}/bin/zerovm -P manifest/"$fileext"-"$DOCCOUNT"-snippet.manifest
		
		cat data/out/"$fileext"_"$DOCCOUNT"_stdout.txt >> result_preview.txt

	fi
	let DOCCOUNT=DOCCOUNT+1
done

cat result_preview.txt



