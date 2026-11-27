export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

# The code is simulated by a metamodel (ddMetamodelRun.py, file given by DD_METAMODEL) : verrou_dd_metamodel
# should find again the failure probabilities, the interactions, the graded clauses and the alternative
# clauses of the metamodel.
#
# run_check (default, about 20s) : 20 deltas without background
#   truthFast.json       : verrou_dd_line, interaction between two clauses
#   truthGradedFast.json : two verrou_dd_sym caches merged, a graded clause (accumulation of small errors,
#                          split by the delta-debug in several ddmin) and the cause a and (b or c)
#                          (alternative clause not found by the delta-debug). The fit is done again
#                          without scipy (noScipy hides it).
# run_check_full (several minutes) : the same checks with 60 deltas and a diffuse background (--background)
.PHONY: clean run_check run_check_full run_check_graded install
all: run_check

FAST_DD=--cache=clean --nruns=5 --num-threads=8

run_check: install
	DD_METAMODEL=$(CURDIR)/truthFast.json verrou_dd_line $(FAST_DD) --seed=2 --rep=dd.fast.line ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.fast.line
	DD_METAMODEL=$(CURDIR)/truthFast.json verrou_dd_metamodel --ratio=3 --nruns=10 dd.fast.line > out.fast.line.metamodel
	./checkFit.py truthFast.json dd.fast.line/metamodel.json
	DD_METAMODEL=$(CURDIR)/truthGradedFast.json verrou_dd_sym $(FAST_DD) --seed=1 --rep=dd.fast.graded1 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.fast.graded1
	DD_METAMODEL=$(CURDIR)/truthGradedFast.json verrou_dd_sym $(FAST_DD) --seed=2 --rep=dd.fast.graded2 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.fast.graded2
	DD_METAMODEL=$(CURDIR)/truthGradedFast.json verrou_dd_metamodel --ratio=3 --nruns=10 dd.fast.graded1 dd.fast.graded2 > out.fast.graded.metamodel
	./checkFit.py --probe truthGradedFast.json dd.fast.graded1/metamodel.json
	PYTHONPATH=$(CURDIR)/noScipy:$$PYTHONPATH verrou_dd_metamodel --output=metamodel.noScipy.json dd.fast.graded1 dd.fast.graded2 > out.fast.noScipy.metamodel
	./checkFit.py --probe truthGradedFast.json metamodel.noScipy.json

run_check_full: install
	verrou_dd_sym --cache=clean --nruns=5 --num-threads=8 --seed=1 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.sym
	verrou_dd_metamodel --background --ratio=4 --nruns=20 dd.sym > out.sym.metamodel
	./checkFit.py truth.json dd.sym/metamodel.json
	verrou_dd_line --cache=clean --nruns=5 --num-threads=8 --seed=2 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.line
	verrou_dd_metamodel --background --ratio=4 --nruns=20 --output=metamodel.line.json dd.line > out.line.metamodel
	./checkFit.py truth.json metamodel.line.json
	$(MAKE) run_check_graded

run_check_graded: export DD_METAMODEL=$(CURDIR)/truthGraded.json
run_check_graded:
	verrou_dd_sym --cache=clean --rep=dd.graded1 --nruns=5 --num-threads=8 --seed=1 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.graded1
	verrou_dd_sym --cache=clean --rep=dd.graded2 --nruns=5 --num-threads=8 --seed=2 ./ddMetamodelRun.py ./ddMetamodelCmp.sh > out.graded2
	verrou_dd_metamodel --background --ratio=4 --nruns=10 dd.graded1 dd.graded2 > out.graded.metamodel
	./checkFit.py --probe truthGraded.json dd.graded1/metamodel.json

install:
	make -C ../.. install

clean:
	rm -rf dd.sym* dd.line* dd.graded* dd.fast.* out.* metamodel.line.json metamodel.noScipy.json noScipy/scipy/__pycache__
