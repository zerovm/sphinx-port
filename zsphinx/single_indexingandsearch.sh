#!/bin/bash


if [ $# -ne 1 ] && [ $# -ne 2 ]
then
    echo "Usage: `basename $0` the file which is added in an index"
    exit $WRONG_ARGS
fi
if [ $# -eq 1 ]
then
    addfile=$1
    query="test"
fi
if [ $# -eq 2 ]
then
    addfile=$1
    query=$2
fi

if ! [ -f manifest ]; then
	mkdir -p manifest
fi

if ! [ -f data/out ]; then
	mkdir -p data/out
fi

if ! [ -f nvram ]; then
	mkdir -p nvram
fi


echo $addfile
echo $query

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH
FILE_NAME=$addfile
CONTETNLENGTH=$(stat -c %s "$FILE_NAME")


echo $query > data/search_stdin.data

echo remove old nexes, outs, manifests and nvrams configuration
rm -f *.nexe
rm -f manifest/*.manifest
rm -f data/out/*
rm -f nvram/*.nvram

if [ -a data/rwindex ]
then
	echo skip index template generating
else
	echo generate zsphinx.conf from template
	./index_template.sh
fi

#echo copy nexes
./copynexe.sh

#generate manifest
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/main_indexer.manifest.template | \
sed s@{FILE_NAME}@$addfile@g > manifest/main_indexer.manifest

#generate nvram
sed s@{CONTETNLENGTH}@$CONTETNLENGTH@g nvram.template/main_indexer.nvram.template | \
sed s@{FILE_NAME}@"$FILE_NAME"@g > nvram/main_indexer.nvram

time ${ZVM_PREFIX}/bin/zerovm -P manifest/main_indexer.manifest

sleep 1

cp data/rwindex_ data/rwindex

cat data/out/indexer_stdout.data

