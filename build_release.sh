#!/bin/bash

if [! -d release ]
then
    mkdir release
fi

cd release
cmake -DSPARQL=1 -DSERVER=1 -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
