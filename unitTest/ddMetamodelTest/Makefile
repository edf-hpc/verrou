export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

# the code is simulated by the metamodel truth.json (ddMetamodelRun.py) : verrou_dd_metamodel should
# find again the failure probabilities and the interaction of truth.json
.PHONY: clean run_check install
all: run_check

run_check: install
	verrou_dd_sym --cache=clean --nruns=5 --num-threads=8 --seed=1 ./ddMetamodelRun.py ./ddMetamodelCmp.py > out.sym
	verrou_dd_metamodel --ratio=4 --nruns=20 dd.sym > out.sym.metamodel
	./checkFit.py truth.json dd.sym/metamodel.json
	verrou_dd_line --cache=clean --nruns=5 --num-threads=8 --seed=2 ./ddMetamodelRun.py ./ddMetamodelCmp.py > out.line
	verrou_dd_metamodel --ratio=4 --nruns=20 --output=metamodel.line.json dd.line > out.line.metamodel
	./checkFit.py truth.json metamodel.line.json

install:
	make -C ../.. install

clean:
	rm -rf dd.sym* dd.line* out.* metamodel.line.json
