#!/bin/bash

dither=(1 2 3 4 5 6 7)
bitness=(1 8)

g++ -std=c++11 -fsanitize=address -fno-stack-limit -o dither main.cpp dithering.cpp

mkdir -p sample
for d in ${dither[@]}; do
    for b in ${bitness[@]}; do
        ./dither ../test/dither.pgm sample/my_1_"$d"_"$b"_1.pgm 1 $d $b 1
        echo "done 1_"$d"_"$b"_1"
    done
done