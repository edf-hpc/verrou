#!/bin/bash
DIR=$1

verrou --exclude=exclude.ex --rounding-mode=random  /usr/bin/python3 ./muller.py> $DIR/res.dat 

verrou --exclude=exclude.ex --rounding-mode=random  ./muller > $DIR/res2.dat

