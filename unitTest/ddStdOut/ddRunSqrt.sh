#!/bin/bash
DIR=$1

if [ "x" = "x$PARAM" ] ; then
    echo "sans PARAM"
    valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random  ./sqrt  > $DIR/res.dat
else
        echo "avec PARAM"
    valgrind --tool=verrou --exclude=exclude.ex --rounding-mode=random  ./sqrt $DIR/$PARAM
fi
