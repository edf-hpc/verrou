SRC=stencil.cpp

BIN=stencil
FLAGS=-Wall -g

BUILDDEP=Makefile
#CPP=clang++
CPP=g++


all: $(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE

$(BIN)-O3-FLOAT: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DFLOAT $(SRC) -o $@

$(BIN)-O3-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O3 -DDOUBLE $(SRC) -o $@

$(BIN)-O0-FLOAT: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DFLOAT $(SRC) -o $@

$(BIN)-O0-DOUBLE: $(SRC) $(BUILDDEP)
	$(CPP) $(FLAGS) -O0 -DDOUBLE $(SRC) -o $@


clean:
	rm $(BIN)-O3-FLOAT $(BIN)-O3-DOUBLE $(BIN)-O0-FLOAT $(BIN)-O0-DOUBLE
