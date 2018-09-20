
.PHONY: clean run install


run: install clean
	VERROU_DD_ALGO=rddmin verrou_dd ./ddRun.py ./ddCmp.py


install:
	make -C .. install

clean:
	rm -rf dd.line* dd.sym* ddmin-*
