.PHONY: build_reference check

include ../flag.mk
FLAGS=${AVX512FLAG} ${FMAFLAG}  -Wall -O0 -g

export INSTALLPATH = $(shell bash ../installpath)
FLAGS += -I$(INSTALLPATH)/include/valgrind
FLAGS += -Wall -Wno-unused
LDFLAGS=-lm

NBSAMPLE=50

check:mpfr_reference check_vprec_rounding
	./check.sh

check_vprec_rounding: check_vprec_rounding.cxx fpPropImpl.c
	g++ $(FLAGS) check_vprec_rounding.cxx -o check_vprec_rounding $(LDFLAGS)

compute_mpfr_rounding: compute_mpfr_rounding.c fpPropImpl.c
	gcc $(FLAGS) compute_mpfr_rounding.c -o compute_mpfr_rounding $(LDFLAGS) -lmpfr

fpProp: fpProp.c fpPropImpl.c
	gcc $(FLAGS) fpProp.c -o fpProp $(LDFLAGS)


build_reference:compute_mpfr_rounding input_11.txt
	rm -rf mpfr_reference
	mkdir mpfr_reference
	./compute_mpfr_rounding ./mpfr_reference
	tar cvfj mpfr_reference.tar.bz2 mpfr_reference

input_%.txt: ./generate_input.py
	./generate_input.py "input_%r.txt" $(NBSAMPLE) 11

mpfr_reference: mpfr_reference.tar.bz2
	tar xfj mpfr_reference.tar.bz2

clean:
	rm -rf ./compute_mpfr_rounding check_vprec_rounding mpfr_reference input_*.txt parallelInput

distclean: clean
	rm -rf mpfr_reference.tar.bz2
