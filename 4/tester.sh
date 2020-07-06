#!/bin/bash

g++ -std=c++11 -fsanitize=address -fno-stack-limit -o convert main.cpp colour_space.cpp

palette=("RGB" "HSL" "HSV" "YCbCr.601" "YCbCr.709" "YCoCg" "CMY")

mkdir -p sample
rm sample/*
for p in ${palette[@]}; do
    echo "running ./convert -f RGB -t $p -i 1 ../test/tree.ppm -o 1 sample/RGB_to_$p.ppm"
    ./convert -f RGB -t $p -i 1 ../test/tree.ppm -o 1 "sample/RGB_to_$p.ppm"
    echo "running ./convert -f RGB -t $p -i 1 ../test/tree.ppm -o 3 sample/RGB_to_$p.pgm"
    ./convert -f RGB -t $p -i 1 ../test/tree.ppm -o 3 "sample/RGB_to_$p.pgm"
    echo "done $p"
done

for p in ${palette[@]}; do
    echo "running ./convert -f $p -t RGB -i 1 sample/RGB_to_$p.pgm -o 1 sample/RGB_to_"$p"_to_RGB_1.ppm"
    ./convert -f $p -t RGB -i 1 "sample/RGB_to_$p.ppm" -o 1 "sample/RGB_to_"$p"_to_RGB_1.ppm"
    echo "running ./convert -f $p -t RGB -i 3 sample/RGB_to_$p.pgm -o 1 sample/RGB_to_"$p"_to_RGB_2.ppm"
    ./convert -f $p -t RGB -i 3 "sample/RGB_to_$p.pgm" -o 1 "sample/RGB_to_"$p"_to_RGB_2.ppm"
    echo "done $p"

    echo "diff -qs ../test/tree.ppm sample/RGB_to_"$p"_to_RGB_1.ppm"
    cmp -l ../test/tree.ppm "sample/RGB_to_"$p"_to_RGB_1.ppm" | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}' | wc -l

    echo "diff -qs ../test/tree.ppm sample/RGB_to_"$p"_to_RGB_2.ppm"
    cmp -l ../test/tree.ppm "sample/RGB_to_"$p"_to_RGB_2.ppm" | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}' | wc -l

    echo "diff -qs sample/RGB_to_"$p"_to_RGB_1.ppm sample/RGB_to_"$p"_to_RGB_2.ppm"
    echo $(diff -qs "sample/RGB_to_"$p"_to_RGB_1.ppm" "sample/RGB_to_"$p"_to_RGB_2.ppm")
    echo ""
done > tester.log
