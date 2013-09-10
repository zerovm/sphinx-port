#!/bin/bash
#Create sphinx config file, to be used by sphinx internals

	#1508
TEMP=$(stat -c %s data/zsphinx.conf)
#TEMP=`cksum data/zsphinx.conf`

echo "{$TEMP index/zsphinx.conf}" > data/rwindex
cat data/zsphinx.conf >> data/rwindex
echo "{index/zsphinx.conf}" >> data/rwindex

for file in mainindex.template/*
do
	FILENAME=`basename $file`
	TEMP1=$(stat -c %s $file)
	echo "{$TEMP1 index/$FILENAME}" >> data/rwindex
	cat $file >> data/rwindex
	echo "{index/$FILENAME}" >> data/rwindex
done

