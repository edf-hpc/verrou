#!/bin/sh
# The results are those of upward or downward, the exclusion of cos does not change exp and cosf
vg="../../vg-in-place -q --tool=verrou --libm=instrumented"
printf 'cos\t*\n' > random-instr-libm.exclude
up=`$vg --rounding-mode=upward ./random-instr-libm print 2>/dev/null | head -1`
down=`$vg --rounding-mode=downward ./random-instr-libm print 2>/dev/null | head -1`
nbUnexpected=0
nbChanged=0
nbExcluded=0
for seed in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
    all=`$vg --rounding-mode=random_instr --vr-seed=$seed ./random-instr-libm print 2>/dev/null | head -1`
    excl=`$vg --rounding-mode=random_instr --vr-seed=$seed --exclude=random-instr-libm.exclude ./random-instr-libm print 2>/dev/null | head -1`
    for field in 1 2 3; do
        value=`echo $all | cut -d' ' -f$field`
        if [ "$value" != "`echo $up | cut -d' ' -f$field`" ] && [ "$value" != "`echo $down | cut -d' ' -f$field`" ]; then
            nbUnexpected=$((nbUnexpected+1))
        fi
    done
    if [ "${all#* }" != "${excl#* }" ]; then nbChanged=$((nbChanged+1)); fi
    if [ "$all" != "$excl" ]; then nbExcluded=$((nbExcluded+1)); fi
done
echo "unexpected results: $nbUnexpected"
echo "direction changed by the exclusion: $nbChanged"
if [ $nbExcluded -gt 0 ]; then echo "exclusion effective"; fi
