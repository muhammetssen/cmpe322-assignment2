#!/bin/bash
make
mkdir -p my_outputs
rm  -f my_outputs/*.txt
for i in {1..5}
do
    ./abstractor.out input_$i.txt my_outputs/o$i.txt
done
