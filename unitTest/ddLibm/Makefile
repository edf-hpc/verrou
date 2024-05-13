export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash


PYTHON=/usr/bin/python3

REP_LIST=dd.sym.random dd.sym.nearest dd.sym.native dd.line.random dd.line.nearest dd.line.native

NBRUN=10

.PHONY: $(REP_LIST)
all: $(REP_LIST)

LIST_UNSTABLE=cos sinf erf atan2 jn
LIST_NATIVE_UNSTABLE=erf jn
LIST_NEAREST_UNSTABLE=erf jn

main_libm: main_libm.cpp
	g++ -O2 -g -fno-builtin main_libm.cpp -o main_libm

dd.sym.random: ./main_libm 
	verrou_dd_sym --cache=clean --rep=$@ --num-threads=5 --nruns=$(NBRUN) --seed=42 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_UNSTABLE}

dd.sym.nearest: ./main_libm 
	CMD_ROUNDING="nearest" verrou_dd_sym --cache=clean --rep=$@ --num-threads=4 --nruns=1 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_NEAREST_UNSTABLE}

dd.sym.native: ./main_libm 
	CMD_ROUNDING="native" verrou_dd_sym --cache=clean --reference-rounding=nearest --rep=$@ --num-threads=4 --nruns=1 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_NATIVE_UNSTABLE}

dd.line.random: ./main_libm 
	verrou_dd_line --cache=clean --rep=$@ --num-threads=5 --nruns=$(NBRUN) --seed=42 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_UNSTABLE}

dd.line.nearest: ./main_libm 
	CMD_ROUNDING="nearest" verrou_dd_line --cache=clean --rep=$@ --num-threads=4 --nruns=1 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_NEAREST_UNSTABLE}

dd.line.native: ./main_libm 
	CMD_ROUNDING="native" verrou_dd_line --cache=clean --reference-rounding=nearest --rep=$@ --num-threads=4 --nruns=1 ddRun.sh cmp.sh
	./assert.py $@ ${LIST_NATIVE_UNSTABLE}

clean:
	rm -rf exclude.ex $(REP_LIST)  main_libm
