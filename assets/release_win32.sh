#!/bin/bash

cd release
# make clean
# rm CMakeCache.txt
cmake .. --toolchain ../tc_mingw_w32.cmake -DCMAKE_BUILD_TYPE='Debug' -DIS_32BIT=TRUE
make -j8

mkdir unscope
mv ./unscope.exe ./unscope/unscope.exe
cp ../LICENSE ./unscope/LICENSE
cp ../submodules/SDL/LICENSE.txt ./unscope/SDL_LICENSE.txt
cp ../README.md ./unscope/README.md
cp ./submodules/SDL/SDL2.dll ./unscope/SDL2d.dll

zip -r ./unscope_win32.zip ./unscope
