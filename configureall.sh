#! /bin/sh

echo -------------------------
echo configure zsphinx
echo -------------------------

export ac_cv_func_malloc_0_nonnull=yes

./configure --host=x86_64-nacl --without-mysql --without-pgsql --without-unixodbc --without-iconv

echo -------------------------
echo configure pdfextractor 
echo -------------------------	
echo 
cd zxpdf-3.03

./configure --host=x86_64-nacl 

