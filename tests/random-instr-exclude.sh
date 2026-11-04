#!/bin/sh
# The direction of siteA does not depend on the exclusion of siteB, excluded sites stay in nearest
printf 'siteB\t*\n' > random-instr.exclude
nbChanged=0
nbExcludedEffect=0
for seed in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
    all=`../../vg-in-place -q --tool=verrou --rounding-mode=random_instr --vr-seed=$seed ./random-instr print 2>/dev/null`
    excl=`../../vg-in-place -q --tool=verrou --rounding-mode=random_instr --vr-seed=$seed --exclude=random-instr.exclude ./random-instr print 2>/dev/null`
    if [ "${all% *}" != "${excl% *}" ]; then nbChanged=$((nbChanged+1)); fi
    if [ "${excl#* }" != "1" ]; then echo "excluded site not in nearest: $excl"; fi
    if [ "$all" != "$excl" ]; then nbExcludedEffect=$((nbExcludedEffect+1)); fi
done
echo "direction changed by the exclusion: $nbChanged"
if [ $nbExcludedEffect -gt 0 ]; then echo "exclusion effective"; fi
