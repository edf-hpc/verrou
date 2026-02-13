.PHONY:

TINYREP=../../../backend_mcaquad/common/
SRC_BACKEND=../../interflop_verrou.cxx
SRC=testProdDenorm.cxx

#CFLAGS=-g -Wall -march=native -DVERROU_NUM_NEARNESS=2 -DVERROU_DET_HASH=vr_double_tabulation_hash
CFLAGS=-O0 -g -Wall -march=native  -DUSE_XOSHIRO -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash -DUSE_VERROU_SQRT -DUSE_VERROU_FMA -DVERROU_DENORM_HACKS_DOUBLE
#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_double_tabulation_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_multiply_shift_hash

#CFLAGS=-O2 -g -Wall -march=native -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_dietzfelbinger_hash


%.o: %.cxx
	g++ $(CFLAGS) -I$(TINYREP) -o $@ -c $<


DEP_OBJ=testProdDenorm.o ../../interflop_verrou.o $(TINYREP)/tinymt64.o
testProdDenorm: $(DEP_OBJ)
	g++ $(CFLAGS) -I$(TINYREP) -o $@ $(DEP_OBJ) -lquadmath

%.o:makefile
testProdDenorm:makefile

clean:
	rm -f $(DEP_OBJ) testProdDenorm
