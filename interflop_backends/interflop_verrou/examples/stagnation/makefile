.PHONY: pdf
TINYREP=../../../backend_mcaquad/common/

SRC=stagnation.cxx ../../interflop_verrou.cxx $(TINYREP)/tinymt64.c

CFLAGS_GEN=-O2 -g -Wall -march=native -DVERROU_DET_HASH=vr_xxhash_hash
CFLAGS=$(CFLAGS_GEN) -DVERROU_NUM_AVG=1



pdf: stagnationErrorLog.pdf stagnationLog.pdf stagnationErrorLog_st.pdf stagnationLog_st.pdf

stagnationErrorLog.pdf: stagnationErrorLog.gp AVERAGE_DET.0.out
	gnuplot stagnationErrorLog.gp

stagnationLog.pdf: stagnationLog.gp AVERAGE_DET.0.out
	gnuplot stagnationLog.gp

stagnationErrorLog_st.pdf: stagnationErrorLogSt.gp AVERAGE_DET.0.out
	gnuplot stagnationErrorLogSt.gp

stagnationLog_st.pdf: stagnationLogSt.gp AVERAGE_DET.0.out
	gnuplot stagnationLogSt.gp


stagnation: $(SRC)
	g++ $(CFLAGS) -I$(TINYREP) -o $@ $(SRC)

AVERAGE_DET.0.out:stagnation
	./stagnation
	./postTreatment.py

%.o:makefile
stagnation:makefile

clean:
	rm -f  stagnation *.out stagnation*.pdf
