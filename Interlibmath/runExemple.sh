#!/bin/sh
echo "random python "
VERROU_LIBM_ROUNDING_MODE=random LD_PRELOAD="./interlibmath.so" ./testCos.py  1.1

echo "average python"
VERROU_ROUNDING_MODE=average LD_PRELOAD="./interlibmath.so" ./testCos.py  1.1

echo "random testCos binary"

VERROU_LIBM_ROUNDING_MODE=random LD_PRELOAD="./interlibmath.so" ./testCos  1.1

echo "native testCos binary"
VERROU_LIBM_ROUNDING_MODE=native LD_PRELOAD="./interlibmath.so" ./testCos  1.1
