#Install Libexpat:

wget http://heanet.dl.sourceforge.net/project/expat/expat/2.1.0/expat-2.1.0.tar.gz;
tar expat-2.1.0.tar.gz;
cd expat-2.1.0;
./configure --host=x86_64-nacl prefix=$ZVM_PREFIX/x86_64-nacl;
make install

#install Zlib:
wget http://zlib.net/zlib-1.2.8.tar.gz;
tar xvf zlib-1.2.8.tar.gz;
cd zlib-1.2.8;
CC=x86_64-nacl-gcc ./configure --prefix=$ZVM_PREFIX/x86_64-nacl;
make;
sudo make install;

##Installing sphinx-port
./configureall.sh; make;
cd zxpdf-3.03; make;
cd ../docxextract make;
cd ../antiword-0.37; make;
cd ../zsphinx;

#Search
./indexingandsearch.sh {~/search path}

#Run sphinx search on zwift
https://github.com/zerovm/sphinx-port/blob/master/zsphinx/zwift/README.md
