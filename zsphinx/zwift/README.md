##How to run sphinx search on swift.

#Preinstalation:
1. Make all nexe files as specified in the instructions https://github.com/zerovm/sphinx-port/blob/master/README.md
2. cd zsphinx/zwift; ./zwiftpreparation.sh {you_account} (for example - g_103319691787805482239)
3. Craete container `"search"` on https://z.litestack.com/js
4. In container `"search"` create next directories: `sys`, `outputfiles`, `doc`(test data).
5. Upload files from local directories ~search/sys, ~search/doc, ~search into same directories on swift.

#Run ndexing:
1. For run indexing - execute `indexing.json`. This json runs the zerovm cluster on swift, who spends indexing of all objects in directory `"swift://{my_account}/search/doc/*"` (like *doc, *.pdf, *.txt, *.docx, *.odt). This path can be specified in part of `"filesender"` node in section `"file_list"` - `"device" : "input"`.
2. execute `merge.json`. It create complete dataindex for search.

#Search
1. For search execute `search.json`. Searched word can be specified in `sys/search_request.txt` object.

