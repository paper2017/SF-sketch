#!/bin/bash

VERSION=$1

if [ "$VERSION"V == SF1V ];then
    echo building for slim-fat sketch version 1
    cmake -DSF1=ON -DSF2=OFF -DSF3=OFF -DSF4=OFF -DSF5=OFF -DMC=OFF .
elif [ "$VERSION"V == SF2V ];then
    echo building for slim-fat sketch version 2
    cmake -DSF1=OFF -DSF2=ON -DSF3=OFF -DSF4=OFF -DSF5=OFF -DMC=OFF .
elif [ "$VERSION"V == SF3V ];then
    echo building for slim-fat sketch version 3
    cmake -DSF1=OFF -DSF2=OFF -DSF3=ON -DSF4=OFF -DSF5=OFF -DMC=OFF .
elif [ "$VERSION"V == SF4V ];then
    echo building for slim-fat sketch version 4
    cmake -DSF1=OFF -DSF2=OFF -DSF3=OFF -DSF4=ON -DSF5=OFF -DMC=OFF .
elif [ "$VERSION"V == SF5V ];then
    echo building for slim-fat sketch version 5
    cmake -DSF1=OFF -DSF2=OFF -DSF3=OFF -DSF4=OFF -DSF5=ON -DMC=OFF .
elif [ "$VERSION"V == SF6V ] || [ "$VERSION"V == V ];then
    echo building for slim-fat sketch version 6
    cmake -DSF1=OFF -DSF2=OFF -DSF3=OFF -DSF4=OFF -DSF5=OFF -DSF6=ON .
else
    echo "usage:  sh ./generate.sh [SF6|SF1|SF2|SF3|SF4|SF5]"
    echo "ps: \"sh ./build.sh\"  for SF6"
    exit
fi

make clean
make
