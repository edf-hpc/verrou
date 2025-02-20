#/bin/bash

if [ "$#" -eq 0 ]; then
    echo "A directory argument is required"
fi

cd $1
diffList=$(find . -name "*std*.diff")

testCasList=${diffList//.stdout.diff/}
testCasList=${testCasList//.stderr.diff/}
testCasList=$(echo $testCasList |uniq)

echo "testCasList:"
echo $testCasList
echo ""

for testCas in $testCasList
do
    echo ""
    echo "TEST CAS: " $testCas

    testCasOutDiff=$testCas".stdout.diff"

    if [ -f $testCasOutDiff ]
    then
	echo $testCasOutDiff
	cat $testCasOutDiff
	echo ""

	testCasOut=$testCas".stdout.out"
	echo $testCasOut
	cat $testCasOut
	echo ""
    fi

    testCasErrDiff=$testCas".stderr.diff"
    if [ -f $testCasErrDiff ]
    then
	echo $testCasErrDiff
	cat $testCasErrDiff
	echo ""

	testCasErr=$testCas".stderr.out"
	echo $testCasErr
	cat $testCasErr
	echo ""
    fi
    echo ""
done
