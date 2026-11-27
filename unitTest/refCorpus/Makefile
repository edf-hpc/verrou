# R1 reference corpus (see README.md).
#   make            : builds the corpus programs in build/
#   make check      : compares to the reference corpus (ref/)
#   make gen        : regenerates ref/ (with the reference verrou-Valgrind)
#   make bench      : performance bench (native, valgrind none, verrou)
# VALGRIND=... : valgrind command (default: valgrind; in the source tree: ../../../vg-in-place)
VALGRIND ?= valgrind
JOBS ?= 4
BUILD = build

all: $(BUILD)/ops $(BUILD)/algos

$(BUILD)/ops: src/ops.c
	mkdir -p $(BUILD)
	gcc -O1 -g -mavx2 -mfma -ffp-contract=off -o $@ $< -lm

$(BUILD)/algos: src/algos.c
	mkdir -p $(BUILD)
	gcc -O2 -g -mavx2 -mfma -o $@ $< -lm -lpthread

check: all
	./corpus.py check --valgrind "$(VALGRIND)" --jobs $(JOBS)

gen: all
	./corpus.py gen --valgrind "$(VALGRIND)" --jobs $(JOBS)

bench: all
	VALGRIND="$(VALGRIND)" ./bench.sh

clean:
	rm -rf $(BUILD) out

.PHONY: all check gen bench clean
