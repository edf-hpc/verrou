#!/bin/bash
OUTDIR=$1
verrou --rounding-mode=random --libm=instrumented --IOmatch-clr=matchLast.script\
	 ./unitTest >${OUTDIR}/res.dat
