#!/bin/bash
DIR=$1

if [ "x" = "x$PARAM" ] ; then
    echo "sans PARAM"
    verrou --exclude=exclude.ex --rounding-mode=random  ./sqrt  > $DIR/res.dat
else
        echo "avec PARAM"
    verrou --exclude=exclude.ex --rounding-mode=random  ./sqrt $DIR/$PARAM
fi
