# R1 reference corpus (see README.md).
#   make            : builds the corpus programs in build/
#   make check      : compares to the reference corpus (ref/)
#   make gen        : regenerates ref/ (with the reference verrou-Valgrind)
#   make bench      : performance bench (native, valgrind none, verrou)
# As the other tests, the installed verrou is used (env.sh of the installation
# path). In the source tree, without installation:
#   make check VALGRIND=$PWD/../../../vg-in-place VERROU_INC="-I../.. -I../../../include"
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash
VALGRIND ?= valgrind
VERROU_INC ?= -I$(INSTALLPATH)/include/valgrind
JOBS ?= 4
BUILD = build

all: $(BUILD)/ops $(BUILD)/algos $(BUILD)/clreq $(BUILD)/naninf $(BUILD)/denorm

$(BUILD)/ops: src/ops.c
	mkdir -p $(BUILD)
	gcc -O1 -g -mavx2 -mfma -ffp-contract=off -o $@ $< -lm

$(BUILD)/algos: src/algos.c
	mkdir -p $(BUILD)
	gcc -O2 -g -mavx2 -mfma -o $@ $< -lm -lpthread

$(BUILD)/clreq: src/clreq.c
	mkdir -p $(BUILD)
	gcc -O1 -g $(VERROU_INC) -o $@ $<

$(BUILD)/naninf: src/naninf.c
	mkdir -p $(BUILD)
	gcc -O1 -g -mavx2 -mfma -ffp-contract=off -o $@ $< -lm

$(BUILD)/denorm: src/denorm.c
	mkdir -p $(BUILD)
	gcc -O1 -g -mavx2 -mfma -ffp-contract=off $(VERROU_INC) -o $@ $<

check: all
	./corpus.py check --valgrind "$(VALGRIND)" --jobs $(JOBS)

gen: all
	./corpus.py gen --valgrind "$(VALGRIND)" --jobs $(JOBS)

bench: all
	VALGRIND="$(VALGRIND)" ./bench.sh

clean:
	rm -rf $(BUILD) out

.PHONY: all check gen bench clean
