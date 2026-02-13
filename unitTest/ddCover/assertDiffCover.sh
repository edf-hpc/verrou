#!/bin/bash

numRun=2
repInteger=dd.line.integer.post
rep=dd.line.post

echo "integer loop"
for i in `seq 0 $numRun`;
do
    for subrep in FullPerturbation rddmin-cmp ddmin0;
    do
	cmd="diff ${repInteger}/${subrep}-trace/nearness/dd.run${i}/cover  ${repInteger}/NoPerturbation-trace/default/dd.run0/cover"
	echo ${cmd}
	${cmd}|| exit 42

    done
done

echo "float loop"
for i in `seq 0 $numRun`;
do

    cmd="diff ${rep}/FullPerturbation-trace/nearness/dd.run${i}/cover  ${rep}/NoPerturbation-trace/default/dd.run0/cover"
    echo ${cmd} "diff expected"
    ! ${cmd} || exit 42

    #we do not check ddmin0 and ddmin1 as we do not known which one is different and which one is equal
    for subrep in rddmin-cmp;
    do
	cmd="diff ${rep}/${subrep}-trace/nearness/dd.run${i}/cover  ${rep}/NoPerturbation-trace/default/dd.run0/cover"
	echo ${cmd}
	${cmd}|| exit 42
    done

done

