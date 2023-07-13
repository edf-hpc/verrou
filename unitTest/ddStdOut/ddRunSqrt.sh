#!/bin/bash
DIR=$1

valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random  ./sqrt> $DIR/res.dat 

