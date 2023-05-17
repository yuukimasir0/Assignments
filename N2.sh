#!/bin/bash

for i in `seq 21 25`
do
    ./compiler samples/ex$i.p
    make asm
    make sr
    echo ""
done

# for i in `seq 21 25`
# do
#     ./compiler samples/failure/ex${i}_fail.p
# done