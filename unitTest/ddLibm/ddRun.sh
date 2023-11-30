#!/bin/bash
DIR=$1

if test "x$CMD_ROUNDING" = "x"
then
    CMD_ROUNDING="random"
fi
valgrind --tool=verrou --rounding-mode=${CMD_ROUNDING} --libm=instrumented ./main_libm > $DIR/res.dat 

