#!/bin/bash

cd release
make clean
rm CMakeCache.txt
cmake .. --toolchain ../tc_mingw.cmake -DCMAKE_BUILD_TYPE='Release'
make -j8
cp ../LICENSE ./LICENSE
cp ../README.md ./README.md
cp ./submodules/SDL/SDL2.dll .
zip ./unscope_win64.zip ./unscope.exe ./LICENSE ./README.md ./SDL2.dll