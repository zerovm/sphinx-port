#!/bin/bash
#Create sphinx config file, to be used by sphinx internals
#1508
echo -e "== \t" generating index template
TEMP=$(stat -c %s ../data/zsphinx.conf)
#TEMP=`cksum data/zsphinx.conf`


printf "        18index/zsphinx.conf%10d" "$TEMP" > search/sys/rwindex
#`cat data/zsphinx.conf` >> data/rwindex___test
#echo -n 'index/zsphinx.conf' >> data/rwindex___test
#TEXTCONF=`cat data/zsphinx.conf`
cat ../data/zsphinx.conf >> search/sys/rwindex
#echo "{index/zsphinx.conf}" >> data/rwindex
#
for file in ../mainindex.template/*
do
	FILENAME=`basename $file`
	TEMP1=$(stat -c %s $file)
	printf "        19index/%s%10d" $FILENAME $TEMP1 >> search/sys/rwindex
#	echo -n $TEMP1'index/'$FILENAME >> data/rwindex___test
	cat $file >> search/sys/rwindex
done

cp search/sys/rwindex 	search/sys/rwindex_copy
