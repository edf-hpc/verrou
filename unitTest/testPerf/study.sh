

PREFIX=" " ./run.sh 2>/dev/null > resNative
PREFIX="valgrind --tool=verrou " ./run.sh 2>/dev/null >resNearest
PREFIX="valgrind --tool=verrou  --rounding-mode=random " ./run.sh 2>/dev/null >resRandom
PREFIX="valgrind --tool=verrou  --rounding-mode=average " ./run.sh 2>/dev/null >resAverage

PREFIX="valgrind --tool=verrou  --rounding-mode=float " ./run.sh 2>/dev/null >resFloat


