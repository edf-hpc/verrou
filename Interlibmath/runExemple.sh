#!/bin/sh
LD_PRELOAD="./interlibmath.so /usr/lib/gcc/i686-linux-gnu/7/libquadmath.so" ./test.py  1.1

LD_PRELOAD="./interlibmath.so /usr/lib/gcc/i686-linux-gnu/7/libquadmath.so" ./testCos  1.1
