

SRC=interlibmath.cxx
FLAGS=-DLIBMATHINTERP -I../backend_verrou -Wall -O0
CXX=g++

interlibmath.so: $(SRC)
	$(CXX) $(SRC) -lquad -lm -shared $(FLAGS) -o interlibmath.so


testCos: testCos.cxx
	$(CXX) -g -Wall -lm  testCos.cxx -o testCos
