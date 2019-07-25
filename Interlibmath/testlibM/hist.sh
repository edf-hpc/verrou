#!/bin/sh

FILE=$1
NUM=$2

index=$(cat $FILE | cut -f $NUM |head -n 1)
echo $index

values=$(cat $FILE | cut -f $NUM | sort -u |grep -v $index)
allvalues=$(cat $FILE | cut -f $NUM | grep -v $index)


for value in $values ;
do
    count=$(cat $FILE | cut -f $NUM | grep -v $index |grep $value |wc -l)
    echo $value , $count
done;
	     

