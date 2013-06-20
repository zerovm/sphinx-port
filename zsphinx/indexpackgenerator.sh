#!/bin/bash
	#1508

#ZSPHCONFSTRING=

ZCONFSIZE=cksum data/zsphinx.conf

echo "{$ZCONFSIZE index/zsphinx.conf}" > rwindex
cat data/zsphinx.conf >> rwindex
echo "{index/zsphinx.conf}"
