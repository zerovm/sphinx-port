# Sphinx port

## Prerequisites

### pcre

    git clone https://github.com/zerovm/zerovm-ports
    cd zerovm-ports/pcre-8.33/
    cat README.md
    <follow instrucations>

### libexpat

    git clone https://github.com/zerovm/zerovm-ports
    cd zerovm-ports/libexpat/
    cat README.md
    <follow instrucations>

### zlib

    git clone https://github.com/zerovm/zerovm-ports
    cd zerovm-ports/zlib/
    cat README.md
    <follow instrucations>

### bzip2

    git clone https://github.com/zerovm/zerovm-ports
    cd zerovm-ports/bzip2/
    cat README.md
    <follow instrucations>



## Installing sphinx-port

    ./configureall.sh; make
    cd zxpdf-3.03; make
    cd ../docxextract make
    cd ../antiword-0.37; make
    cd ../zsphinx

## Usage

    ./indexingandsearch.sh {~/search path}

#Run sphinx search on zwift
https://github.com/zerovm/sphinx-port/blob/master/zsphinx/zwift/README.md


[zerovm-ports]: https://github.com/zerovm/zerovm-ports