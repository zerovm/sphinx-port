#!/bin/bash
echo -e "== \t" generating index template
#1508
TEMP=$(stat -c %s ../data/zsphinx.conf)
#TEMP=`cksum data/zsphinx.conf`

rm -f search/data/rwindex

echo "{$TEMP index/zsphinx.conf}" > search/sys/rwindex
cat ../data/zsphinx.conf >> search/sys/rwindex
echo "{index/zsphinx.conf}" >> search/sys/rwindex

for file in ../mainindex.template/*
do
	FILENAME=`basename $file`
	TEMP1=$(stat -c %s $file)
	echo "{$TEMP1 index/$FILENAME}" >> search/sys/rwindex
	cat $file >> search/sys/rwindex
	echo "{index/$FILENAME}" >> search/sys/rwindex
done

