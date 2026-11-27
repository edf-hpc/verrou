#!/bin/bash
# Performance bench of the R1 corpus: stencil of unitTest/testPerf (compute
# time measured by the program, without startup, minimum over the
# iterations), native, under valgrind --tool=none and under verrou (several
# rounding modes).
# Usage: VALGRIND=... ./bench.sh [file.csv]   (default: out/bench.csv)
# The times depend on the machine: they are not compared automatically.
set -u
cd "$(dirname "$0")"
VALGRIND=${VALGRIND:-valgrind}
PERF=../testPerf
OUT=${1:-out/bench.csv}
mkdir -p "$(dirname "$OUT")"
VARIANTS="stencil-O3-DOUBLE stencil-O3-DOUBLE-FMA stencil-O0-DOUBLE stencil-O3-FLOAT"
make -s -C $PERF $VARIANTS > /dev/null || exit 1
mintime() { "$@" 2>/dev/null | sed -n 's/.*@mintime of serial run:.*\[\(.*\)\].*/\1/p'; }
{
    echo "# cpu: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//')"
    echo "# $($VALGRIND --version 2>/dev/null) ; $(gcc --version | head -1)"
    echo "variant,configuration,seconds"
} > "$OUT"
printf "%-24s %-22s %9s\n" variant configuration seconds
for v in $VARIANTS; do
    B=$PERF/$v
    for c in "native" "none" "nearest" "random" "random_det" "float"; do
        case $c in
            native) t=$(mintime $B --scale=0.5 3) ;;
            none) t=$(mintime $VALGRIND --tool=none $B --scale=0.5 3) ;;
            *) t=$(mintime $VALGRIND --tool=verrou --rounding-mode=$c --vr-seed=42 $B --scale=0.5 3) ;;
        esac
        printf "%-24s %-22s %9s\n" $v $c "$t"
        echo "$v,$c,$t" >> "$OUT"
    done
done
echo "results: $OUT"
