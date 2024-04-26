export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

all: dd.synchro

dd.synchro: clean
	verrou_dd_synchro --num-threads=4 --nrun=1 ddRun.sh extractOrCmp.py

clean:
	rm -rf exclude.ex dd.synchro
