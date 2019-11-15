#!/bin/sh


if [ -f gcc-9.2.0/bin/g++ ]; then
    echo "GCC found -- nothing to build."
else
    echo "Building GCC 9.2.0"
    wget https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
    tar -xf gcc-9.2.0.tar.xz
    rm gcc-9.2.0.tar.xz
    cd gcc-9.2.0
    ./contrib/download_prerequisites
    cd .. && mkdir gcc-9.2.0-objdir && cd gcc-9.2.0-objdir
    $PWD/../gcc-9.2.0/configure --prefix=$PWD/../gcc-9 --exec-prefix=$PWD/../gcc-9 --enable-languages=c,c++
    make -j $(nproc) all
    make install
    cd .. && rm gcc-9.2.0-objdir && rm gcc-9.2.0
fi

export PATH=$PATH:gcc-9/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:gcc-9/lib64
