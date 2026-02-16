#!/bin/bash
OUTDIR=$1

valgrind --tool=verrou --backend=vprec --vprec-preset=fp24 \
	 ./unitTestInteger >${OUTDIR}/res.dat
