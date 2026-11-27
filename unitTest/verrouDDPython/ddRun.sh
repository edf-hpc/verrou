#!/bin/bash
DIR=$1
verrou --rounding-mode=downward  trace_verrou_task.py ./Muller.py > $DIR/res.dat

