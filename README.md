# Sphinx port

## Prerequisites

Install in the following order

1. [zlib](https://github.com/zerovm/zerovm-ports/blob/master/zlib/README.md)
2. [bzip2](https://github.com/zerovm/zerovm-ports/blob/master/bzip2/README.md)
3. [libexpat](https://github.com/zerovm/zerovm-ports/blob/master/libexpat/README.md)
4. [pcre](https://github.com/zerovm/zerovm-ports/blob/master/pcre-8.33/README.md)
5. [iconv](https://github.com/zerovm/zerovm-ports/blob/master/libiconv/README.md)

## Building sphinx-port

    ./configure --host=x86_64-nacl --disable-i18n --without-mysql --without-pgsql --without-unixodbc --without-iconv
    make


## Usage

    ./indexingandsearch.sh {~/search path}

# Run sphinx search on zwift

[README](https://github.com/zerovm/sphinx-port/blob/master/zsphinx/zwift/README.md)

