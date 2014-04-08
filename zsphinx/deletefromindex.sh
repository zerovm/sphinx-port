#!/bin/bash

echo copy nexes
./copynexe.sh


#./allmanifestgenerator.sh "$directory"

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/xmlpipecreator_delete.manifest.template > manifest/xmlpipecreator_delete.manifest
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/indexer.manifest.template > manifest/indexer.manifest

cp nvram.template/xml_delete.nvram.template nvram/xml_delete.nvram
cp nvram.template/indexer.nvram.template nvram/indexer.nvram

#Run nameserver and pass cluster nodes count as param
./ns_start.sh 2

${ZVM_PREFIX}/bin/zerovm -P manifest/xmlpipecreator_delete.manifest &
${ZVM_PREFIX}/bin/zerovm -P manifest/indexer.manifest

./ns_stop.sh

cat data/out/indexer_stdout.data


