#!/bin/bash
#{ACCOUNTID} google account ID

if [ $# -ne 1 ]
then
	echo "Usage: `basename $0` {account id}"
 	exit $WRONG_ARGS
else
	ACCOUNTID=$1
fi

echo -e "== \t" preparing jsons
rm -f search/*.json
sed s@{my_account}@$ACCOUNTID@g json.templates/indexing.json.template > search/indexing.json
sed s@{my_account}@$ACCOUNTID@g json.templates/merge.json.template > search/merge.json
sed s@{my_account}@$ACCOUNTID@g json.templates/search.json.template > search/search.json


