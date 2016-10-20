#!/bin/bash

VERSION=$1

if [ "$VERSION"V == SF1V ];then
    echo building for slim-fat sketch version 1
    cmake -DSF1=ON -DSF2=OFF -DSF3=OFF -DSF4=OFF -DSFF=OFF .
elif [ "$VERSION"V == SF2V ];then
    echo building for slim-fat sketch version 2
    cmake -DSF1=OFF -DSF2=ON -DSF3=OFF -DSF4=OFF -DSFF=OFF .
elif [ "$VERSION"V == SF3V ];then
    echo building for slim-fat sketch version 3
    cmake -DSF1=OFF -DSF2=OFF -DSF3=ON -DSF4=OFF -DSFF=OFF .
elif [ "$VERSION"V == SF4V ];then
    echo building for slim-fat sketch version 4
    cmake -DSF1=OFF -DSF2=OFF -DSF3=OFF -DSF4=ON -DSFF=OFF .
elif [ "$VERSION"V == SFFV ] || [ "$VERSION"V == V ];then
    echo building for slim-fat sketch version 6
    cmake -DSF1=OFF -DSF2=OFF -DSF3=OFF -DSF4=OFF -DSFF=ON .
else
    echo "usage:  sh ./build.sh [SFF|SF1|SF2|SF3|SF4]"
    echo "ps: \"sh ./build.sh\"  for SFF"
    exit
fi

make clean
make
