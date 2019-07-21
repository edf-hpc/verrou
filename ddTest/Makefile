
.PHONY: clean run install
all:run_line run_sym

run_line: install clean
	VERROU_DD_ALGO=rddmin verrou_dd_line ./ddRun.py ./ddCmp.py
run_sym: install clean
	VERROU_DD_ALGO=rddmin verrou_dd_sym ./ddRun.py ./ddCmp.py


install:
	make -C .. install

clean:
	rm -rf dd.line* dd.sym* ddmin-*
