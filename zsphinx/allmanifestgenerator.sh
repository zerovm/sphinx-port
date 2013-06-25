#!/bin/bash
	#1508

if [ $# -ne 1 ]
then
  echo "Usage: `basename $0` search_path"
  exit $WRONG_ARGS
else
directory=$1
fi

FILECOUNT=3 #1 - indexer, 2-xmlpipecreator, 3 - нод первого документа 4 - и т.д. 
PDFCOUNT=1
TXTCOUNT=1
DOCCOUNT=1
DOCXCOUNT=1
ODTCOUNT=1
SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

mkdir manifest
mkdir data/out
mkdir nvram


#удаляем старые манифесты
rm -f manifest/*.manifest

#удаляем старые nvram.conf
rm -f nvram/*.nvram

#готовим новый манифест для xmlpipecreator
sed s@{ABS_PATH}@$ABS_PATH/@ manifest.template/xmlpipecreator.manifest.template > manifest/xmlpipecreator.manifest

echo ===================================================================== >> manifest/xmlpipecreator.manifest
echo "==Channels for connected input nodes" >> manifest/xmlpipecreator.manifest
echo ===================================================================== >> manifest/xmlpipecreator.manifest



#генерация манифеста для indexer
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/indexer.manifest.template > manifest/indexer.manifest

#генерация манифеста для search
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/search.manifest.template > manifest/search.manifest

#генерируем nvram для indexer, search, xmlpipecreator
cp nvram.template/xml.nvram.template nvram/xml.nvram
cp nvram.template/search.nvram.template nvram/search.nvram
cp nvram.template/indexer.nvram.template nvram/indexer.nvram


for file in $directory/*
do
	filename=${file%.}
	fileext="${file##*.}"
	echo $file
	echo $filename
	echo $fileext
	
	if [ "$fileext" = "txt" ]
	then
		#echo "test all manifest generator **txt** " $FILECOUNT $filename $TXTCOUNT
		./nodemanifestgenerator.sh $FILECOUNT "$filename" $TXTCOUNT
		./nodenvramgenerator.sh $FILECOUNT "$filename" $TXTCOUNT
		echo $filename $fileext
		DEVICENAME=/dev/in/$fileext
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,"$DEVICENAME-$TXTCOUNT", 0, 0, 99999999, 99999999, 0, 0"
		let FILECOUNT=FILECOUNT+1
		let TXTCOUNT=TXTCOUNT+1
		echo $CHANNELNAME >> manifest/xmlpipecreator.manifest
	fi
	if [ "$fileext" = "pdf" ] 
	then 
		#echo "test all manifest generator **pdf** " $FILECOUNT $filename $TXTCOUNT
		./nodemanifestgenerator.sh $FILECOUNT "$filename" $PDFCOUNT
		./nodenvramgenerator.sh $FILECOUNT "$filename" $PDFCOUNT
		echo $filename $fileext
		DEVICENAME=/dev/in/$fileext
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,"$DEVICENAME-$PDFCOUNT", 0, 0, 99999999, 99999999, 0, 0"
		let FILECOUNT=FILECOUNT+1
		let PDFCOUNT=PDFCOUNT+1
		echo $CHANNELNAME >> manifest/xmlpipecreator.manifest
	fi
	if [ "$fileext" = "doc" ] 
	then 
		#echo "test all manifest generator **doc** " $FILECOUNT $filename $TXTCOUNT
		./nodemanifestgenerator.sh $FILECOUNT "$filename" $DOCCOUNT
		./nodenvramgenerator.sh $FILECOUNT "$filename" $DOCCOUNT
		echo $filename $fileext
		DEVICENAME=/dev/in/$fileext
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,"$DEVICENAME-$DOCCOUNT", 0, 0, 99999999, 99999999, 0, 0"
		let FILECOUNT=FILECOUNT+1
		let DOCCOUNT=DOCCOUNT+1
		echo $CHANNELNAME >> manifest/xmlpipecreator.manifest
	fi
	if [ "$fileext" = "docx" ] 
	then 
		#echo "test all manifest generator **docx** " $FILECOUNT $filename $TXTCOUNT
		./nodemanifestgenerator.sh $FILECOUNT "$filename" $DOCXCOUNT
		./nodenvramgenerator.sh $FILECOUNT "$filename" $DOCXCOUNT
		echo $filename $fileext
		DEVICENAME=/dev/in/$fileext
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,"$DEVICENAME-$DOCXCOUNT", 0, 0, 99999999, 99999999, 0, 0"
		let FILECOUNT=FILECOUNT+1
		let DOCXCOUNT=DOCXCOUNT+1
		echo $CHANNELNAME >> manifest/xmlpipecreator.manifest
	fi
	if [ "$fileext" = "odt" ] 
	then 
		#echo "test all manifest generator **docx** " $FILECOUNT $filename $TXTCOUNT
		./nodemanifestgenerator.sh $FILECOUNT "$filename" $ODTCOUNT
		./nodenvramgenerator.sh $FILECOUNT "$filename" $ODTCOUNT
		echo $filename $fileext
		DEVICENAME=/dev/in/$fileext
		CHANNELNAME="Channel = tcp:"$FILECOUNT":,"$DEVICENAME-$ODTCOUNT", 0, 0,  99999999, 99999999, 0, 0"
		let FILECOUNT=FILECOUNT+1
		let ODTCOUNT=ODTCOUNT+1
		echo $CHANNELNAME >> manifest/xmlpipecreator.manifest
	fi
done

