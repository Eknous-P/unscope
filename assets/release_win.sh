#!/bin/bash

cd release
make clean
rm CMakeCache.txt
cmake .. --toolchain ../tc_mingw.cmake -DCMAKE_BUILD_TYPE='Release'
make -j8

mkdir unscope
mv ./unscope.exe ./unscope/unscope.exe
cp ../LICENSE ./unscope/LICENSE
cp ../submodules/SDL/LICENSE.txt ./unscope/SDL_LICENSE.txt
cp ../README.md ./unscope/README.md
cp ./submodules/SDL/SDL2.dll ./unscope/

zip -r ./unscope_win64.zip ./unscope
