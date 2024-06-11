export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash


PYTHON=/usr/bin/python3

REP_LIST=dd.stdout dd.stdout.cpp dd.stdout.empty dd.stdout.debug dd.stdout.debug2 dd.stdout.sqrt dd.stdout.sqrt.listing dd.stdout.regroup dd.task
#dd.stdout.emptyBis 
NBRUN=5

.PHONY: $(REP_LIST)
all: $(REP_LIST)

exclude.ex:
	./genExcludePython.sh $(PYTHON) exclude.ex

muller: muller.cpp
	g++ -O2 -g muller.cpp -o muller

muller-task: muller.cpp
	g++ -O2 -DWITH_VERROU_TASK -g muller.cpp -lverrouTask -o muller-task

sqrt: sqrt.cxx
	g++ -O2 -g sqrt.cxx -o sqrt

dd.stdout: exclude.ex 
	CMD=./muller.py	verrou_dd_stdout --cache=clean --num-threads=5 --nruns=$(NBRUN) --seed=42 ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout

dd.stdout.cpp: ./muller exclude.ex
	CMD="./muller" verrou_dd_stdout --rep=dd.stdout.cpp --num-threads=5 --nruns=$(NBRUN) --seed=42  ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.cpp

dd.stdout.empty: ./muller exclude.ex
	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.empty --num-threads=5 --nruns=$(NBRUN) --seed=42 ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.empty

#dd.stdout.emptyBis: ./muller exclude.ex
#	CMD="./muller emptyLine" verrou_dd_stdout --rep=dd.stdout.emptyBis --num-threads=6 --nruns=$(NBRUN) --ignore-empty-line=False ddRun.sh extractOrCmp.py
#	./assertDDMuller.py dd.stdout.emptyBis emptyLine

dd.stdout.debug: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug --num-threads=6 --nruns=$(NBRUN) --seed=42 --ignore-empty-line=True --filter-cmd=./pythonFilter.py  ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.debug

dd.stdout.debug2: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.debug2 --num-threads=6 --nruns=$(NBRUN) --seed=42 --ignore-empty-line=True --filter-cmd="/usr/bin/sed -u s/debug.*//" ddRun.sh extractOrCmp.py
	./assertDDMuller.py dd.stdout.debug2

dd.stdout.fusion: ./muller exclude.ex
	CMD="./muller debugPrev debugPost" verrou_dd_stdout --rep=dd.stdout.fusion --num-threads=6 --nruns=$(NBRUN) --seed=42 --ignore-empty-line=True --filter-cmd="./pythonFilter.py --loop-fusion=4" ddRun.sh extractOrCmp.py
	./assertDDFusion.py dd.stdout.regroup

dd.stdout.sqrt: ./sqrt exclude.ex
	verrou_dd_stdout --rep=dd.stdout.sqrt --num-threads=5 --nruns=$(NBRUN) --seed=123 --IOmatch-header=iomatch.header --custom-search-space=iomatch.custom ddRunSqrt.sh extractOrCmpSqrt.py
	./assertSqrt.py dd.stdout.sqrt

dd.stdout.sqrt.listing: ./sqrt exclude.ex
	PARAM=listing verrou_dd_stdout --rep=dd.stdout.sqrt.listing --num-threads=5 --nruns=$(NBRUN) --seed=123 --IOmatch-header=iomatch.header --custom-search-space=iomatch.custom.listing --file-pattern=*listing  ddRunSqrt.sh extractOrCmpSqrt.py
	./assertSqrt.py dd.stdout.sqrt.listing

dd.task: ./muller-task
	CMD="./muller-task" verrou_dd_task --rep=dd.task --num-threads=6 --nruns=$(NBRUN) --seed=42  ddRun.sh extractOrCmp.py
	./assertTask.py dd.task


clean:
	rm -rf exclude.ex $(REP_LIST)  muller sqrt muller-task
