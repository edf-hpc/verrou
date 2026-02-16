#!/bin/sh



for i in  dotConditionNumber dotConditionNumberWithoutAssert dotConditionNumberRewrite dotConditionNumberWithoutAssertRewrite
do
    export BIN_NAME=./$i
    cmd="verrou_plot_stat --samples=10 --rep=stat.$i --rounding=random,random_det,random_comdet,nearness,nearness_det,nearness_comdet runVerrou.sh extract.sh"
    echo $cmd
    $cmd
done
