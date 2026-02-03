#!/bin/bash


#modelist="ob ib full"
modelist="ob"
typeList="float double"



for mode in ${modelist} ;
do
    for realType in ${typeList};
    do
	if [ ${realType} = "float" ]
	then
	   bit=32
	   rangelist=`seq 4 8`
	   precisionlist=`seq 4 23`
	fi

	if [ ${realType} = "double" ]
	then
	   bit=64
	   rangelist=`seq 5 11`
	   precisionlist=`seq 4 52`
	fi

    

    for range in ${rangelist} ;
    do
	for precision in ${precisionlist} ;
	do

	       
	    valGrind="valgrind --tool=verrou --backend=vprec --vprec-range-binary${bit}=${range} --vprec-precision-binary${bit}=${precision} --vprec-mode=${mode} --count-op=no --quiet --check-nan=no --check-inf=no"
	    fileRef="./mpfr_reference/mpfr_${realType}_${mode}_E${range}M${precision} "
	    runCmd="${valGrind} ./check_vprec_rounding ${fileRef} ${range} ${precision}"
	    #echo ${realType} ${mode} E${range} M${precision} ${valGrind} ${runCmd}
	    echo ${runCmd}
	    ${runCmd} ||  exit 42;
	done
    done		    
    done
done
