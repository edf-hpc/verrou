

PREFIX=" " ./run.sh 2>/dev/null > resNative
PREFIX="verrou " ./run.sh 2>/dev/null >resNearest
PREFIX="verrou  --rounding-mode=random " ./run.sh 2>/dev/null >resRandom
PREFIX="verrou  --rounding-mode=nearness " ./run.sh 2>/dev/null >resNearness

PREFIX="verrou  --rounding-mode=float " ./run.sh 2>/dev/null >resFloat


