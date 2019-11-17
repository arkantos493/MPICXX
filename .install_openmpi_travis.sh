#!/bin/sh

if [ -f openmpi/lib/libmpi.so ]; then
  echo "OpenMPI found -- nothing to build."
else

echo "Downloading OpenMPI source"
wget https://download.open-mpi.org/release/open-mpi/v4.0/openmpi-4.0.2.tar.gz
gunzip -c openmpi-4.0.2.tar.gz | tar xf -
rm openmpi-4.0.2.tar.gz
cd openmpi-4.0.2
echo "Building OpenMPI"
./configure --prefix=/home/openmpi --exec-prefix=/home/openmpi
make -j $(nproc)
sudo make install
cd ..
rm -rf openmpi-4.0.2
export PATH=$PATH:/home/openmpi/bin
