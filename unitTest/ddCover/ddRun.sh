#!/bin/bash
OUTDIR=$1
valgrind --tool=verrou --rounding-mode=random --libm=instrumented\
	 ./unitTest >${OUTDIR}/res.dat
