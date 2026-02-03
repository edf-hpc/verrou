
FLAGS=-g -Wall -O0 -mfma
export INSTALLPATH = $(shell bash ../installpath)
FLAGS += -I$(INSTALLPATH)/include/valgrind
FLAGS += -Wall -Wno-unused
LDFLAGS=-lm

NBSAMPLE=10

check:mpfr_reference check_vprec_rounding
	./check.sh

check_vprec_rounding: check_vprec_rounding.cxx fpPropImpl.c
	g++ $(FLAGS) check_vprec_rounding.cxx -o check_vprec_rounding $(LDFLAGS)

compute_mpfr_rounding: compute_mpfr_rounding.c
	gcc $(FLAGS) compute_mpfr_rounding.c -o compute_mpfr_rounding $(LDFLAGS) -lmpfr

fpProp: fpProp.c fpPropImpl.c
	gcc $(FLAGS) fpProp.c -o fpProp $(LDFLAGS)


mpfr_reference.tar.gz:compute_mpfr_rounding input_11.txt
	rm -rf mpfr_reference
	mkdir mpfr_reference
	./compute_mpfr_rounding ./mpfr_reference
	tar cvfz mpfr_reference.tar.gz mpfr_reference

input_%.txt: ./generate_input.py
	./generate_input.py "input_%r.txt" $(NBSAMPLE) 11

input_float.txt: ./generate_input.py
	./generate_input.py input_float.txt $(NBSAMPLE) 8

input_double.txt: ./generate_input.py
	./generate_input.py input_double.txt $(NBSAMPLE) 11


mpfr_reference: mpfr_reference.tar.gz
	tar xfz mpfr_reference.tar.gz

clean:
	rm -rf ./compute_mpfr_rounding check_vprec_rounding mpfr_reference mpfr_reference.tar.gz input_*.txt
