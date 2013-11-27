#!/bin/bash

TEMP=$(stat -c %s data/zsphinx.conf)

printf "        18index/zsphinx.conf%10d" "$TEMP" > data/rwindex_conf

cat data/zsphinx.conf >> data/rwindex_conf

