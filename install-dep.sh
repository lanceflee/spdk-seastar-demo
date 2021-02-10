#!/bin/bash

ROOT_DIR=$(pwd)
SPDK_SOURCE_DIR=${ROOT_DIR}/spdk
SEASTAR_SOURCE_DIR=${ROOT_DIR}/seastar
DPDK_SOURCE_DIR=${ROOT_DIR}/seastar/dpdk

git submodule update --init --recursive


#build dpdk
#d ${DPDK_SOURCE_DIR}
#make config T=x86_64-native-linuxapp-gcc
#make -j24

#build seastar 
cd ${SEASTAR_SOURCE_DIR}
#add --cook fmt if needed
#./configure.py --mode=release --enable-dpdk
./configure.py --mode=release --cook fmt
ninja -C build/release

#build spdk
cd ${SPDK_SOURCE_DIR}
./scripts/pkgdep.sh

#./configure --without-isal --without-vhost --disable-tests --with-dpdk=${DPDK_SOURCE_DIR}/build
#IMPORTANT!!! DONOT use static lib!!!
./configure --without-isal --without-vhost --disable-tests --with-shared
make -j24
