#/bin/bash

if [ "$#" -eq 0 ]; then
    echo "A directory argument is required"
fi

cd $1
diffList=$(find . -name "*stdout.diff")

for diff in $diffList
do
    stdout=${diff/stdout.diff/stdout.out}
    stderr=${diff/stdout.diff/stderr.out}

    echo $diff
    cat $diff
    echo ""
    echo $stdout
    cat $stdout
    echo ""
    echo $stderr
    cat  $stderr

    echo ""
done
