#!/bin/bash

echo copy nexes
cp ../src/xmlpipecreator 	xmlpipecreator.nexe
cp ../src/indexer 		indexer.nexe

#./allmanifestgenerator.sh "$directory"

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
ABS_PATH=$SCRIPT_PATH

sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/xmlpipecreator_index_template.manifest.template > manifest/xmlpipecreator_index_template.manifest
sed s@{ABS_PATH}@$ABS_PATH/@g manifest.template/indexer_index_template.manifest.template > manifest/indexer_index_template.manifest

cp nvram.template/xml_index_template.nvram.template nvram/xml_index_template.nvram
cp nvram.template/indexer_index_template.nvram.template nvram/indexer_index_template.nvram

if [ -a data/rwindex_conf ]
then
	echo skip index template generating
else
	echo generate zsphinx.conf from template
	./rwgenerator_.sh
fi

#Run nameserver and pass cluster nodes count as param
./ns_start.sh 2

${ZVM_PREFIX}/bin/zerovm -P manifest/xmlpipecreator_index_template.manifest &
${ZVM_PREFIX}/bin/zerovm -P manifest/indexer_index_template.manifest

./ns_stop.sh


cat data/out/indexer_stdout.data


