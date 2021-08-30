#! /bin/bash

rm -rf build
mkdir build && cd build
cmake ../
make common_lib

make -j4