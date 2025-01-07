#!/bin/bash

rm -rf ./release/*
cd release
cmake .. -DCMAKE_BUILD_TYPE='Release'
make -j8
cp ../LICENSE ./LICENSE
cp ../README.md ./README.md
zip ./unscope.zip ./unscope ./LICENSE ./README.md