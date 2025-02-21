#!/bin/bash

make clean
cd release
cmake .. --toolchain ../tc_mingw.cmake -DCMAKE_BUILD_TYPE='Release'
make -j8
cp ../LICENSE ./LICENSE
cp ../README.md ./README.md
zip ./unscope_win64.zip ./unscope.exe ./LICENSE ./README.md ./submodules/SDL/SDL2.dll