
.PHONY: clean run install
all:run_line run_sym

run_line: install
	VERROU_DD_CACHE=clean VERROU_DD_ALGO=rddmin verrou_dd_line --num-threads=4 ./ddRun.py ./ddCmp.py
run_sym: install
	VERROU_DD_CACHE=clean VERROU_DD_ALGO=rddmin verrou_dd_sym ./ddRun.py ./ddCmp.py


install:
	make -C .. install

clean:
	rm -rf dd.line* dd.sym* ddmin-*
