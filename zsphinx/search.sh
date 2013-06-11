#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

rm -f manifest/search.manifest
rm -f data/out/search_stdout.data

cp ../src/search 		search.nexe

#генерация манифеста для search
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/search.manifest.template > manifest/search.manifest

${ZVM_PREFIX}/zerovm -Mmanifest/search.manifest 
cat data/out/search_stdout.data

