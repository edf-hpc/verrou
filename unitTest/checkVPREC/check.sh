#!/bin/bash

. $INSTALLPATH/env.sh

modelist="ob ib full"
modelist="ob full" #which ulp for tolerance with ib?
#modelist="ob"
typeList="float double"
nbSample=100
inc=1


for mode in ${modelist} ;
do
    for realType in ${typeList};
    do
	if [ ${realType} = "float" ]
	then
	   bit=32
	   rangelist=`seq 4 ${inc} 8`
	   precisionlist=`seq 3 ${inc} 23`
	fi

	if [ ${realType} = "double" ]
	then
	   bit=64
	   rangelist=`seq 5 ${inc} 11`
	   precisionlist=`seq 4 ${inc} 52`
	fi

    
    runListFile="parallelInput"
    echo "" > ${runListFile}
    for range in ${rangelist} ;
    do
	for precision in ${precisionlist} ;
	do

	       
	    valGrind="valgrind --tool=verrou --backend=vprec --vprec-range-binary${bit}=${range} --vprec-precision-binary${bit}=${precision} --vprec-mode=${mode} --count-op=no --quiet --check-nan=no --check-inf=no"
	    fileRef="./mpfr_reference/mpfr_${realType}_${mode}_E${range}M${precision} "
	    runCmd="${valGrind} ./check_vprec_rounding ${fileRef} ${range} ${precision} ${nbSample}"
	    #echo ${realType} ${mode} E${range} M${precision} ${valGrind} ${runCmd}
	    echo ${runCmd} >> ${runListFile}
	    #${runCmd} ||  exit 42;
	done	
    done

    parallel --group --halt now,fail=1 -j 5  :::: ${runListFile} || exit 42
    
    done    
done
