
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash


.PHONY: clean run install
all:run_check

run_check: install
	OUTCMD="| tee out && ./ddCheck.py dd.sym out && test ${PIPESTATUS[0]} -eq 0 "
	verrou_dd_sym --cache=clean --num-thread=4 ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_sym --cache=clean --nruns=2 ./ddRun.py ./ddCmp.py  ${OUTCMD}
	verrou_dd_sym --num-thread=2 --cache=keep_run ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_sym --cache=rename --rddmin-heuristics-cache=cache  ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_sym --cache=continue --res-with-all-samples --nruns=5 ./ddRun.py ./ddCmp.py ${OUTCMD}

	verrou_dd_line --cache=clean --num-thread=5 ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_line --cache=clean --num-thread=3 --nruns=2 ./ddRun.py ./ddCmp.py  ${OUTCMD}
	verrou_dd_line --num-thread=6 --cache=keep_run --nruns=2 ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_line --cache=rename --rddmin-heuristics-cache=cache --nruns=2 --num-thread=6 ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_line --cache=rename --rddmin-heuristics-cache=cache --rddmin-heuristics-line-conv --nruns=2 --num-thread=6 ./ddRun.py ./ddCmp.py ${OUTCMD}
	verrou_dd_line --cache=continue --res-with-all-samples --nruns=5 --num-thread=6 ./ddRun.py ./ddCmp.py ${OUTCMD}



install:
	make -C ../.. install

clean:
	rm -rf dd.line* dd.sym* ddmin-*
