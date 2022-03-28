

SRC=interlibmath.cxx ../interflop_backends/backend_mcaquad/common/tinymt64.c
FLAGS= -std=c++11 -DLIBMATHINTERP -I../backend_verrou -Wall -O0
#FLAGS+=-DCLIENTREQUEST #not required if the user include libgcc_s.so 

CXX=g++

interlibmath.so: $(SRC)
	$(CXX) $(FLAGS) $(SRC) -lquadmath -lm -ldl -shared -fPIC   -o interlibmath.so


testCos: testCos.cxx
	$(CXX) -g -Wall -lm  testCos.cxx -o testCos

clean:
	rm -f testCos interlibmath.so
