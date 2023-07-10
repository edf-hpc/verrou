PYTHON=/usr/bin/python3
all: dd.stdout dd.stdout.cpp dd.stdout.empty dd.stdout.emptyBis

exclude.ex:
	./genExcludePython.sh $(PYTHON) exclude.ex

muller: muller.cpp
	g++ -O2 -g muller.cpp -o muller

dd.stdout: exclude.ex 
	CMD=./muller.py	verrou_dd_stdout --cache=clean --num-threads=5 --nruns=15 ddRun.sh extractOrCmp.py

dd.stdout.cpp: ./muller exclude.ex
	CMD="./muller" verrou_dd_stdout --rep=dd.stdout.cpp --num-threads=5 --nruns=15 ddRun.sh extractOrCmp.py

dd.stdout.empty: ./muller exclude.ex
	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.empty --num-threads=5 --nruns=15 ddRun.sh extractOrCmp.py

dd.stdout.emptyBis: ./muller exclude.ex
	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.emptyBis --num-threads=6 --nruns=15 --ignore-empty-line=False ddRun.sh extractOrCmp.py

dd.stdout.debug: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug --num-threads=6 --nruns=15 --ignore-empty-line=True --filter-cmd=./pythonFilter.py  ddRun.sh extractOrCmp.py

dd.stdout.debug2: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug2 --num-threads=6 --nruns=15 --ignore-empty-line=True --filter-cmd="sed s/debug.*//" ddRun.sh extractOrCmp.py



clean:
	rm -rf exclude.ex dd.stdout dd.stdout.cpp  dd.stdout.empty dd.stdout.emptyBis  muller
