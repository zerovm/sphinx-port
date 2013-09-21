#!/bin/bash
#Create sphinx config file, to be used by sphinx internals

#1508
TEMP=$(stat -c %s data/zsphinx.conf)
#TEMP=`cksum data/zsphinx.conf`

rm -f data/rwindex
cp data/rwindex_copy data/rwindex

