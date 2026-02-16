SRC=stencil_interflop_verrou.cpp

SRC_VERROU=../../interflop_verrou.cxx ../../../backend_mcaquad/common/tinymt64.c 
FLAGS_VERROU=-DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash -DVERROU_IGNORE_NANINF_CHECK -DUSE_VERROU_FMA -mfma -march=native -DUSE_VERROU_SQRT

BIN=stencil_interflop_verrou

FLAGS=-Wall -g $(FLAGS_VERROU) -fno-exceptions -fno-rtti -std=c++11

BUILDDEP=Makefile
CPP=clang++
CPP=g++

BIN_LIST=$(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE $(BIN)-O2-FLOAT $(BIN)-O2-DOUBLE
all: $(BIN_LIST)

#always recompile to avoid problem with missing deps
.PHONY: $(BIN_LIST) 

$(BIN)-O3-FLOAT: $(SRC) $(SRC_VERROU) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DFLOAT $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O3-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DDOUBLE $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O2-FLOAT: $(SRC) $(SRC_VERROU) $(BUILDDEP)
	$(CPP) $(FLAGS) -O2 -DFLOAT $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O2-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O2 -DDOUBLE $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O0-FLOAT: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DFLOAT $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O0-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DDOUBLE $(SRC)  $(SRC_VERROU) -o $@


clean:
	rm $(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE
