PYTHON=/usr/bin/python3
all: dd.synchro

exclude.ex:
	./genExcludePython.sh $(PYTHON) exclude.ex


dd.synchro: clean exclude.ex
	verrou_dd_synchro ddRun.sh extractOrCmp.py

clean:
	rm -rf exclude.ex dd.synchro
