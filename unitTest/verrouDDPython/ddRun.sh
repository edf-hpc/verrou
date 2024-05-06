#!/bin/bash
DIR=$1
valgrind --tool=verrou --rounding-mode=downward  trace_verrou_task.py ./Muller.py > $DIR/res.dat

