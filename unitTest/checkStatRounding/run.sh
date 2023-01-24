#!/bin/bash

EXEC_PATH=$(dirname $0)

valgrind --tool=verrou --vr-verbose=no --rounding-mode=random ${EXEC_PATH}/checkStatRounding > $1/res.out
