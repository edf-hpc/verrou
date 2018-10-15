#!/bin/sh
LD_PRELOAD="./interlibmath.so" ./testCos.py  1.1

LD_PRELOAD="./interlibmath.so" ./testCos  1.1
