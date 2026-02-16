

SRC=interlibmath.cxx ../interflop_backends/backend_mcaquad/common/tinymt64.c
FLAGS= -std=c++11 -DLIBMATHINTERP -DINTERLIBM_STAND_ALONE -I../backend_verrou -Wall -O2 -DVERROU_NUM_NEARNESS=1 -DVERROU_DET_HASH=vr_xxhash_hash  -DUSE_VERROU_SQRT  -DUSE_VERROU_FMA -fno-builtin -Wno-strict-aliasing

CXX=g++

FLAGQUADMATH=
LDFLAGQUADMATH=

PLATFORM = $(shell uname -m)
ifeq ($(PLATFORM),aarch64)
	FLAGQUADMATH=
	LDFLAGQUADMATH=
endif
ifeq ($(PLATFORM),x86_64)
	LDFLAGQUADMATH=-lquadmath
	FLAGQUADMATH=-DUSE_VERROU_QUADMATH
endif


interlibmath.so: $(SRC) stacktrace.cxx
	$(CXX) $(FLAGS) $(SRC) $(FLAGQUADMATH) -lm $(LDFLAGQUADMATH) -ldl  -shared -fPIC   -o interlibmath.so


testCos: testCos.cxx
	$(CXX) -g -Wall -lm  testCos.cxx -o testCos

clean:
	rm -f testCos interlibmath.so
