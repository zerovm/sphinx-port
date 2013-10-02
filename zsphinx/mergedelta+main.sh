#!/bin/bash
#генерация манифеста для indexer

#удаляем старые манифесты
rm -f manifest/*.manifest
#удаляем старые nvram.conf
rm -f nvram/*.nvram
#
rm -f data/out/*

cp ../src/indexer 		indexer.nexe

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

#генерируем манифесты
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/indexer_merge.manifest.template > manifest/indexer_merge.manifest
#генерируем nvram
cp nvram.template/indexer_merge.nvram.template nvram/indexer_merge.nvram

#запускаем
echo run manifest/indexer_merge.manifest
time ${ZVM_PREFIX}/bin/zerovm -P -Mmanifest/indexer_merge.manifest

sleep 1

cat data/out/indexer_stdout.data

