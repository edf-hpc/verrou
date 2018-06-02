export INSTALLPATH = $(shell bash ../installpath)
export VALGRIND=$(INSTALLPATH)/bin/valgrind --tool=verrou 

FLAGS=-Wall -O0 -g -march=native -fabi-version=0 -std=c++11 -mavx

SOURCE=testFloat.cxx
EXEC=./testFloat

.PHONY: clean run runValgrind runFenv

run: runFenv runValgrind

$(EXEC): $(SOURCE) Makefile
	$(CXX) $(FLAGS) $(LDFLAGS) $(SOURCE) -o $(EXEC)


runFenv:$(EXEC)
	$(EXEC) fenv --rounding-mode=nearest
	$(EXEC) fenv --rounding-mode=upward
	$(EXEC) fenv --rounding-mode=downward
	$(EXEC) fenv --rounding-mode=toward_zero

runValgrind:$(EXEC)
	$(VALGRIND) --rounding-mode=nearest     $(EXEC) valgrind --rounding-mode=nearest	 
	$(VALGRIND) --rounding-mode=upward	 $(EXEC) valgrind --rounding-mode=upward	 
	$(VALGRIND) --rounding-mode=downward    $(EXEC) valgrind --rounding-mode=downward	 
	$(VALGRIND) --rounding-mode=toward_zero $(EXEC) valgrind --rounding-mode=toward_zero 

clean:
	rm $(EXEC)
