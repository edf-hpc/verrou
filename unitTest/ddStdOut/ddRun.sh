#!/bin/bash
DIR=$1

if test "x$CMD" = "x"
then
    CMD="/usr/bin/python3 ./muller.py"
fi
valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random --vr-instr-scalar=yes ${CMD}> $DIR/res.dat 

