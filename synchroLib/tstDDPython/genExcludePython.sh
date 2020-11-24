#!/bin/sh

PYTHON_BIN=`readlink -f $1`
OUT_FILE=$2
echo "$PYTHON_BIN"

echo "∗" `ldd ${PYTHON_BIN} | grep libm | cut -f 3 -d " " | xargs readlink -f ` > ${OUT_FILE}

echo "_Py_HashDouble $PYTHON_BIN" >> ${OUT_FILE}   
echo "_Py_dg_strtod  $PYTHON_BIN" >> ${OUT_FILE}   
