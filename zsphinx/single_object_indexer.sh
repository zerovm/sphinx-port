#!/bin/bash
#{DOCNUMBER} порядковый номер документа
#{FILE_NAME} имя подключаемого файла

if [ $# -lt 1 ]
then
	echo "Usage: `basename $0` {FILE_NAME}"
 	exit $WRONG_ARGS
fi

FILE_NAME=$1
ARGS=$2

if ! [ -f manifest ]; then
	mkdir -p manifest
fi

if ! [ -f data/out ]; then
	mkdir -p data/out
fi

if ! [ -f nvram ]; then
	mkdir -p nvram
fi

if [ -a data/presettings.tar ]
then
	echo data/presettings.tar already exist
else
	echo create data/presettings.tar
	cd data
	tar -cvvf presettings.tar charsets/ antiword/ usr/
	cd ..
fi


SCRIPT=$(readlink -f "$0")
ABS_PATH=`dirname "$SCRIPT"`/

rm -rf manifest/*.manifest

echo "generate manifest"
sed s@{ABS_PATH}@$ABS_PATH@ manifest.template/single_object_indexer.manifest.template | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > manifest/single_object_indexer.manifest

echo "generate nvram"
sed s@{ABS_PATH}@$ABS_PATH@ nvram.template/single_object_indexer.nvram.template | \
sed s@{ARGS}@"$ARGS"@g | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > nvram/single_object_indexer.nvram

time zerovm manifest/single_object_indexer.manifest

mv data/saved_index.data data/rwindex
