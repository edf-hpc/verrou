#!/bin/bash
# Checks that "verrou OPTIONS PROG ARGS" behaves as
# "valgrind --tool=verrou OPTIONS PROG ARGS".
# Usage: ./check.sh PROG   (verrou and valgrind of the PATH, as installed)
PROG=$1
fail=0
ok() { echo "OK    $1"; }
ko() { echo "KO    $1"; fail=1; }
norm() { sed -e 's/^==[0-9]*==/==PID==/' -e 's/^--[0-9]*--/--PID--/' -e '/Command:/d' -e '/First seed/d'; }

# same outputs (numerical results and verrou log) for several options
for opts in "" "--rounding-mode=upward" "--rounding-mode=random_det --vr-seed=1" "-q --count-op=no --rounding-mode=float"; do
    verrou $opts $PROG 0 a b > out.verrou 2> err.verrou
    rv=$?
    valgrind --tool=verrou $opts $PROG 0 a b > out.valgrind 2> err.valgrind
    rg=$?
    if [ $rv -eq $rg ] && cmp -s out.verrou out.valgrind && cmp -s <(norm < err.verrou) <(norm < err.valgrind); then
        ok "same output as valgrind --tool=verrou [$opts]"
    else
        ko "different output from valgrind --tool=verrou [$opts]"
        diff out.verrou out.valgrind | head -5
        diff <(norm < err.verrou) <(norm < err.valgrind) | head -5
    fi
done

# exit code of the program
verrou -q $PROG 3 > /dev/null 2>&1
[ $? -eq 3 ] && ok "exit code of the program (3)" || ko "exit code of the program"

# --tool is refused before the program...
verrou --tool=memcheck $PROG > /dev/null 2> err.tool
[ $? -eq 1 ] && grep -q "is not accepted" err.tool && ok "--tool refused" || ko "--tool refused"
# ...but it is an argument of the program after it
verrou -q $PROG 0 --tool=x > out.arg 2> /dev/null
grep -q "^arg --tool=x$" out.arg && ok "--tool=x after the program is passed to it" || ko "--tool=x after the program"

# VERROU_VALGRIND selects the valgrind command
cat > fake_valgrind <<'FAKE'
#!/bin/sh
echo "fake $*"
FAKE
chmod +x fake_valgrind
out=$(VERROU_VALGRIND=$PWD/fake_valgrind verrou --rounding-mode=upward ./x y)
[ "$out" = "fake --tool=verrou --rounding-mode=upward ./x y" ] && ok "VERROU_VALGRIND" || ko "VERROU_VALGRIND: $out"

# the valgrind installed next to the command is preferred to the PATH one
mkdir -p bin
cp "$(command -v verrou)" bin/verrou
cp fake_valgrind bin/valgrind
out=$(PATH=/usr/bin:/bin bin/verrou ./x)
[ "$out" = "fake --tool=verrou ./x" ] && ok "valgrind next to the command" || ko "valgrind next to the command: $out"

exit $fail
