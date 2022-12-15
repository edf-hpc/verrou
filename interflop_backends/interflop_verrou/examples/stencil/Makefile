SRC=stencil_interflop_verrou.cpp

SRC_VERROU=../../interflop_verrou.cxx ../../../backend_mcaquad/common/tinymt64.c 
FLAGS_VERROU=-DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_double_tabulation_hash -DVERROU_IGNORE_NANINF_CHECK

BIN=stencil_interflop_verrou
FLAGS=-Wall -g $(FLAGS_VERROU)

BUILDDEP=Makefile
#CPP=clang++
CPP=g++


all: $(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE

$(BIN)-O3-FLOAT: $(SRC) $(SRC_VERROU) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DFLOAT $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O3-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DDOUBLE $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O0-FLOAT: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DFLOAT $(SRC)  $(SRC_VERROU) -o $@

$(BIN)-O0-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DDOUBLE $(SRC)  $(SRC_VERROU) -o $@


clean:
	rm $(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE
