#!/bin/bash



for i in `seq 1 100` ;
do
    verrou --rounding-mode=$1 ./a.out || exit 42
done
