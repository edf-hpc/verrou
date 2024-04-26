#!/bin/bash
DIR=$1
valgrind --tool=verrou --rounding-mode=downward  trace_verrou_synchro.py ./Muller.py > $DIR/res.dat

