#!/bin/bash
OUTDIR=$1

verrou --backend=vprec --vprec-preset=fp24 \
	 ./unitTestInteger >${OUTDIR}/res.dat
