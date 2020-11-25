#!/bin/bash
DIR=$1

valgrind --demangle=no --tool=verrou --exclude=exclude.ex --rounding-mode=random  /usr/bin/python3 ../trace_verrou_synchro.py ./Muller.py > $DIR/res.dat

