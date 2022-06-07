#!/bin/bash

EXEC_PATH=$(dirname $0)

valgrind --tool=verrou ${EXEC_PATH}/checkStatRounding > $1/res.out
