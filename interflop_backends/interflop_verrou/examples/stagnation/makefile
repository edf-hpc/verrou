.PHONY: pdf
TINYREP=../../../backend_mcaquad/common/

SRC_BACKEND=../../interflop_verrou.cxx

SRC=stagnation.cxx

#CFLAGS=-g -Wall -march=native -DVERROU_NUM_AVG=2 -DVERROU_DET_HASH=vr_double_tabulation_hash
CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_mersenne_twister_hash
#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_xxhash_hash



pdf: stagnationErrorLog.pdf stagnationLog.pdf stagnationErrorLog_st.pdf stagnationLog_st.pdf

stagnationErrorLog.pdf: stagnationErrorLog.gp AVERAGE_DET.out
	gnuplot stagnationErrorLog.gp

stagnationLog.pdf: stagnationLog.gp AVERAGE_DET.out
	gnuplot stagnationLog.gp

stagnationErrorLog_st.pdf: stagnationErrorLogSt.gp AVERAGE_DET.out
	gnuplot stagnationErrorLogSt.gp

stagnationLog_st.pdf: stagnationLogSt.gp AVERAGE_DET.out
	gnuplot stagnationLogSt.gp




%.o: %.cxx
	g++ $(CFLAGS) -I$(TINYREP) -o $@ -c $<


DEP_OBJ=stagnation.o ../../interflop_verrou.o $(TINYREP)/tinymt64.o
stagnation: $(DEP_OBJ)
	g++ $(CFLAGS) -I$(TINYREP) -o $@ $(DEP_OBJ)

AVERAGE_DET.out:stagnation
	./stagnation

%.o:makefile
stagnation:makefile

clean:
	rm -f *.o stagnation *.out stagnation*.pdf
