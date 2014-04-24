# Sphinx port

## Prerequisites

Install in the following order

### zlib

[README](https://github.com/zerovm/zerovm-ports/blob/master/zlib/README.md)

### bzip2

[README](https://github.com/zerovm/zerovm-ports/blob/master/bzip2/README.md)

### libexpat

[README](https://github.com/zerovm/zerovm-ports/blob/master/libexpat/README.md)

### pcre

[README](https://github.com/zerovm/zerovm-ports/blob/master/pcre-8.33/README.md)

## Installing sphinx-port

    ./configureall.sh; make
    cd zxpdf-3.03; make
    cd ../docxextract make
    cd ../antiword-0.37; make
    cd ../zsphinx

## Usage

    ./indexingandsearch.sh {~/search path}

#Run sphinx search on zwift

[README](https://github.com/zerovm/sphinx-port/blob/master/zsphinx/zwift/README.md)


[zerovm-ports]: https://github.com/zerovm/zerovm-ports
