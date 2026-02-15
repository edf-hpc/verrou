.PHONY: pdf
TINYREP=../../../backend_mcaquad/common/

SRC_BACKEND=../../interflop_verrou.cxx

SRC=testadiva.cxx

#CFLAGS=-g -Wall -march=native -DVERROU_NUM_NEARNESS=2 -DVERROU_DET_HASH=vr_double_tabulation_hash
CFLAGS=-O2 -g -Wall -march=native  -DUSE_XOSHIRO -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash
#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_double_tabulation_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_multiply_shift_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_dietzfelbinger_hash


%.o: %.cxx
	g++ $(CFLAGS) -I$(TINYREP) -o $@ -c $<


DEP_OBJ=testadiva.o ../../interflop_verrou.o $(TINYREP)/tinymt64.o
testadiva: $(DEP_OBJ)
	g++ $(CFLAGS) -I$(TINYREP) -o $@ $(DEP_OBJ)

%.o:makefile
testadiva:makefile

clean:
	rm -f $(DEP_OBJ) testadiva
