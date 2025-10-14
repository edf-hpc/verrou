
SRC=test_main.cxx interflop_verrou.cxx ../backend_mcaquad/common/tinymt64.c

CFLAGS=-g -Wall -march=native -O2 -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_xxhash_hash  -DUSE_VERROU_FMA -DUSE_VERROU_SQRT


test_main: $(SRC)
	g++ $(CFLAGS) -o $@ $^

clean:
	rm test_main
