#!/bin/bash

make clean
cd release
cmake .. -DCMAKE_BUILD_TYPE='Release'
make -j8
cp ../LICENSE ./LICENSE
cp ../README.md ./README.md
zip ./unscope_linux.zip ./unscope ./LICENSE ./README.md