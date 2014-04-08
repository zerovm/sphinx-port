#!/bin/bash


./rwgenerator.sh > result.data

LIMIT=20
for ((a = 2; a <= LIMIT; a++))
do
	echo path to search /home/volodymyr/disk/$a-f1
	./indexingandsearch.sh /home/volodymyr/disk/$a-f1
	./mergedelta+main.sh
done	

./search.sh telephone

