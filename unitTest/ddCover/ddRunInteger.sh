#!/bin/bash
OUTDIR=$1

#workaround to deactivate instrumented libm option when VERROU_COUNT_DENORM is active
#if [[ -z "${VERROU_COUNT_DENORM}" ]]; then
    LIBMOPTION=instrumented
#else
#    LIBMOPTION=manual_exclude
#fi

valgrind --tool=verrou --rounding-mode=random --libm=$LIBMOPTION\
	 ./unitTestInteger >${OUTDIR}/res.dat
