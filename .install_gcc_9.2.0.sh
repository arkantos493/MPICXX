#!/bin/sh


if [ -f gcc-9.2.0/bin/g++ ]; then
    echo "GCC found -- nothing to build."
else
    echo "Building GCC 9.2.0"
    echo "  -- downloading GCC 9.2.0"
    wget https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
    echo "  -- unpacking GCC 9.2.0"
    tar xf gcc-9.2.0.tar.xz
    rm gcc-9.2.0.tar.xz
    cd gcc-9.2.0 && ls -a && ls -a contrib
    echo "  -- installing prerequisites"
   # ./contrib/download_prerequisites
    cd .. && mkdir gcc-9.2.0-objdir && cd gcc-9.2.0-objdir
    echo "  -- configure GCC 9.2.0"
    "$PWD/../gcc-9.2.0/configure" --prefix="/home/gcc-9" --exec-prefix="/home/gcc-9" --enable-languages=c,c++
    echo " -- build and install GCC 9.2.0"
    make -j $(nproc) all
    make install
    cd .. && rm gcc-9.2.0-objdir && rm gcc-9.2.0
fi

echo "Setting environment variables"
export PATH=$PATH:/home/gcc-9/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/gcc-9/lib64
