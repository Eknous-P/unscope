#!/bin/bash

cd release || exit 1
make clean
rm -rf ./*
cmake .. -DCMAKE_BUILD_TYPE='Release'
make -j8

mkdir unscope
mv ./unscope ./unscope/unscope
cp ../LICENSE ./unscope/LICENSE
cp ../README.md ./unscope/README.md

zip -r ./unscope_linux.zip ./unscope
exit 0
