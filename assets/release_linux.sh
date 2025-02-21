#!/bin/bash

cd release
make clean
rm CMakeCache.txt
cmake .. -DCMAKE_BUILD_TYPE='Release'
make -j8
cp ../LICENSE ./LICENSE
cp ../README.md ./README.md
zip ./unscope_linux.zip ./unscope ./LICENSE ./README.md