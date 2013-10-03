#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

if [ $# -eq 1 ]
then
query=$1
echo $query > data/search_stdin.data
fi

rm -f manifest/search.manifest
rm -f data/out/search_stdout.data

cp ../src/search 		search.nexe

#генерация манифеста для search
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/search.manifest.template > manifest/search.manifest

#генерация nvram для search
#cp nvram.template/search.nvram.template nvram/search.nvram
sed s@{command}@"$query"@g nvram.template/search.nvram.template > nvram/search.nvram


time ${ZVM_PREFIX}/bin/zerovm -P -Mmanifest/search.manifest -T /home/volodymyr/searchlog -s
cat data/out/search_stdout.data

