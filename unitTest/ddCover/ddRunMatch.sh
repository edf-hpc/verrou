#!/bin/bash
OUTDIR=$1
valgrind --tool=verrou --rounding-mode=random --libm=instrumented --IOmatch-clr=matchLast.script\
	 ./unitTest >${OUTDIR}/res.dat
