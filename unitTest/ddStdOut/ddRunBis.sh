#!/bin/bash
DIR=$1

valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random  /usr/bin/python3 ./muller.py> $DIR/res.dat 

valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random  ./muller > $DIR/res2.dat

