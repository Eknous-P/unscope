#!/bin/bash

cd release
make clean
rm CMakeCache.txt
cmake .. -DCMAKE_BUILD_TYPE='Release'
make -j8

mkdir unscope
mv ./unscope ./unscope/unscope
cp ../LICENSE ./unscope/LICENSE
cp ../README.md ./unscope/README.md

zip -r ./unscope_linux.zip ./unscope
