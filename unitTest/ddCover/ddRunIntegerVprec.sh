#!/bin/bash
OUTDIR=$1

valgrind --tool=verrou --backend=vprec --vprec-preset=fp16 \
	 ./unitTestInteger >${OUTDIR}/res.dat
