#!/bin/bash
make
for i in `seq 21 25`
do
    ./compiler samples/ex$i.p
    make asm
    make sr
    echo ""
done

./compiler samples/pi.p
make asm
make sr
echo ""

# for i in `seq 21 25`
# do
#     ./compiler samples/failure/ex${i}_fail.p
# done