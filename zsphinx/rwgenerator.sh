#!/bin/bash
	#1508
TEMP=$(stat -c %s data/zsphinx.conf)
#TEMP=`cksum data/zsphinx.conf`

echo "{$TEMP index/zsphinx.conf}" > data/rwindex
cat data/zsphinx.conf >> data/rwindex
echo "{index/zsphinx.conf}" >> data/rwindex
