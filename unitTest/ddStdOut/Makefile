export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash


PYTHON=/usr/bin/python3

REP_LIST=dd.stdout dd.stdout.cpp dd.stdout.empty dd.stdout.emptyBis dd.stdout.debug dd.stdout.debug2
NBRUN=15

.PHONY: $(REP_LIST)
all: $(REP_LIST)

exclude.ex:
	./genExcludePython.sh $(PYTHON) exclude.ex

muller: muller.cpp
	g++ -O2 -g muller.cpp -o muller

dd.stdout: exclude.ex 
	CMD=./muller.py	verrou_dd_stdout --cache=clean --num-threads=5 --nruns=$(NBRUN) ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout

dd.stdout.cpp: ./muller exclude.ex
	CMD="./muller" verrou_dd_stdout --rep=dd.stdout.cpp --num-threads=5 --nruns=$(NBRUN) ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.cpp

dd.stdout.empty: ./muller exclude.ex
	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.empty --num-threads=5 --nruns=$(NBRUN) ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.empty

dd.stdout.emptyBis: ./muller exclude.ex
	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.emptyBis --num-threads=6 --nruns=$(NBRUN) --ignore-empty-line=False ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.emptyBis emptyLine

dd.stdout.debug: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug --num-threads=6 --nruns=$(NBRUN) --ignore-empty-line=True --filter-cmd=./pythonFilter.py  ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.debug

dd.stdout.debug2: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug2 --num-threads=6 --nruns=$(NBRUN) --ignore-empty-line=True --filter-cmd="sed s/debug.*//" ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.debug2



clean:
	rm -rf exclude.ex $(REP_LIST)  muller
