#!/bin/sh

file=`tempfile --prefix=xlsview --suffix=.html`

cat << EOT >$file
<HTML>
<HEAD>
<TITLE>$1</TITLE>
</HEAD>

<BODY BGCOLOR="#FFFFFF" >
<TABLE border=1>
EOT

xls2csv "$1" | while read line
do

echo "<tr>" >>$file
echo $line | awk -F, '{print "<td><font size=2>" $1 "</td><td><font size=2>"  $2 "</td><td><font size=2>"  $3 "</td><td><font size=2>"  $4 "</td><td><font size=2>"  $5 "</td><td><font size=2>"  $6 "</td><td><font size=2>"  $7 "</td><td><font size=2>"  $8 "</td><td><font size=2>"  $9 "</td><td><font size=2>"  $10 "</td><td><font size=2>"  $11 "</td><td><font size=2>"  $12 "</td>" }' >>$file
 
echo "</tr>" >>$file

done

echo "</table>" >>$file

#Try to open file in an existing netscape window
(netscape -remote "openFile(${file})") > /dev/null 2>&1 || rm $file

#if this fails, it means that netscape is not running, so start it
if [ $status ]; then
  netscape -no-install file:${file}
fi

