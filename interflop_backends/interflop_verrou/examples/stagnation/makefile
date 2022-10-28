SRC_BACKEND=../../interflop_verrou.cxx

SRC=stagnation.cxx

#CFLAGS=-g -Wall -march=native -DVERROU_NUM_AVG=2 -DVERROU_DET_HASH=vr_double_tabulation_hash
CFLAGS=-g -Wall -march=native -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_mersenne_twister_hash

stagnationErrorLog.pdf: stagnationLog.gp AVERAGE_DET.out
	gnuplot stagnationLog.gp
%.o: %.cxx
	g++ -o $@ -c $< $(CFLAGS)


DEP_OBJ=stagnation.o ../../interflop_verrou.o ../../../backend_mcaquad/common/tinymt64.o
stagnation: $(DEP_OBJ)
	g++ -o $@ $(DEP_OBJ) $(CFLAGS)

AVERAGE_DET.out:stagnation
	./stagnation

%.o:makefile
stagnation:makefile

clean:
	rm *.o stagnation *.out stagnationErrorLog.pdf 
