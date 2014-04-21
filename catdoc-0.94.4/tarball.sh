#!/bin/sh

# This v.ugly hack is going away
# once we get autoconf updated properly

set -e

test ! -d .pc

VERSION=0.94.4
#debclean
find . -name '*.o' -delete
rm -f config.cache config.log config.status Makefile src/catppt src/xls2csv
rm -f doc/Makefile charsets/Makefile build-stamp install-stamp src/catdoc src/wordview src/Makefile
cd ../
if [ -h catdoc-${VERSION} ]; then
  rm catdoc-${VERSION}
fi
ln -s ./wheezy/ catdoc-${VERSION}
tar -czf catdoc-${VERSION}.tar.gz ./catdoc-${VERSION}/* --exclude=debian --exclude=.svn
ln -sf catdoc-${VERSION}.tar.gz catdoc_${VERSION}.orig.tar.gz

