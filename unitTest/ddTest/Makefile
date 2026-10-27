
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash


# check the exit status of verrou_dd_* then its result with ddCheck.py
CHECK_SYM = | tee out; test $${PIPESTATUS[0]} -eq 0 && ./ddCheck.py dd.sym out
CHECK_LINE = | tee out; test $${PIPESTATUS[0]} -eq 0 && ./ddCheck.py dd.line out

.PHONY: clean run install
all:run_check

run_check: install
	verrou_dd_sym --cache=clean --num-thread=4 ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=clean --nruns=2 ./ddRun.py ./ddCmp.py  $(CHECK_SYM)
	verrou_dd_sym --cache=clean --rddmin=stoch --nruns=2 --num-thread=4 ./ddRun.py ./ddCmp.py  $(CHECK_SYM)
	verrou_dd_sym --cache=clean --rddmin=strict --nruns=2 --num-thread=4 ./ddRun.py ./ddCmp.py  $(CHECK_SYM)
	verrou_dd_sym --num-thread=2 --cache=keep_run ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=rename --rddmin-heuristics-cache=cache  ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=continue --res-with-all-samples --nruns=5 ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=clean --algo=ddmax --nruns=5 ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=clean --algo=ddmax --nruns=4 --num-thread=4 ./ddRun.py ./ddCmp.py $(CHECK_SYM)
	verrou_dd_sym --cache=clean --rddmin=adaptive --nruns=4 ./ddRun.py ./ddCmp.py  $(CHECK_SYM)
	verrou_dd_sym --cache=clean --rddmin=dicho-adaptive --nruns=4 --num-thread=4 --seed=42 ./ddRun.py ./ddCmp.py  $(CHECK_SYM)

	verrou_dd_line --cache=clean --num-thread=5 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=clean --num-thread=3 --nruns=2 ./ddRun.py ./ddCmp.py  $(CHECK_LINE)
	verrou_dd_line --num-thread=6 --cache=keep_run --nruns=2 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=rename --rddmin-heuristics-cache=cache --nruns=2 --num-thread=6 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=rename --rddmin-heuristics-cache=cache --rddmin-heuristics-line-conv --nruns=2 --num-thread=6 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=continue --res-with-all-samples --nruns=5 --num-thread=6 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=clean --algo=ddmax --nruns=5 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=clean --algo=ddmax --nruns=4 --num-thread=4 ./ddRun.py ./ddCmp.py $(CHECK_LINE)
	verrou_dd_line --cache=clean --rddmin=adaptive --nruns=4 --num-thread=4 ./ddRun.py ./ddCmp.py  $(CHECK_LINE)
	verrou_dd_line --cache=clean --rddmin=dicho-adaptive --nruns=4 --seed=42 ./ddRun.py ./ddCmp.py  $(CHECK_LINE)


install:
	make -C ../.. install

clean:
	rm -rf dd.line* dd.sym* ddmin-* out
