

SRC=interlibmath.cxx ../interflop_backends/backend_mcaquad/common/tinymt64.c
FLAGS= -std=c++11 -DLIBMATHINTERP -DINTERLIBM_STAND_ALONE -I../backend_verrou -Wall -O2 -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_xxhash_hash

CXX=g++

interlibmath.so: $(SRC) stacktrace.cxx
	$(CXX) $(FLAGS) $(SRC) -lquadmath -lm -ldl -shared -fPIC   -o interlibmath.so


testCos: testCos.cxx
	$(CXX) -g -Wall -lm  testCos.cxx -o testCos

clean:
	rm -f testCos interlibmath.so
