#!/bin/bash
# Client request API: verrou.h self-contained (C and C++), version request,
# values returned natively and under verrou.
fail=0
check() { # name expected actual
    if [ "$2" == "$3" ]; then echo "OK    $1"; else echo "KO    $1: '$3' instead of '$2'"; fail=1; fi
}
for prog in ./api ./api_cxx; do
    out=$($prog)
    check "$prog native: version 0" "api_version 0" "$(grep ^api_version <<< "$out")"
    check "$prog native: rounding 0" "rounding 0" "$(grep ^rounding <<< "$out")"
    out=$(verrou --rounding-mode=upward --vr-seed=42 $prog 2> /dev/null)
    header=$(grep ^VERROU_API_VERSION <<< "$out" | cut -d' ' -f2)
    check "$prog verrou: version of the header" "api_version $header" "$(grep ^api_version <<< "$out")"
    check "$prog verrou: rounding upward (VR_UPWARD=1)" "rounding 1" "$(grep ^rounding <<< "$out")"
    check "$prog verrou: double instrumented" "is_instrumented_double 1" "$(grep ^is_instrumented <<< "$out")"
    check "$prog verrou: seed" "seed 42" "$(grep ^seed <<< "$out")"
    check "$prog verrou: set rounding in lower case (VR_DOWNWARD=2)" "set_lower 2" "$(grep ^set_lower <<< "$out")"
    check "$prog verrou: set rounding in upper case (VR_ZERO=3)" "set_upper 3" "$(grep ^set_upper <<< "$out")"
    check "$prog verrou: invalid rounding kept" "set_invalid 3" "$(grep ^set_invalid <<< "$out")"
done
exit $fail
