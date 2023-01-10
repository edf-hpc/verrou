SRC_BACKEND=interflop_verrou.cxx

SRC=test_main.cxx

CFLAGS=-g -Wall -march=native -DVERROU_NUM_AVG=1 -DVERROU_DET_HASH=vr_double_tabulation_hash  -DUSE_VERROU_FMA  -O2


%.o: %.cxx
	g++ -o $@ -c $< $(CFLAGS)


test_main: test_main.o interflop_verrou.o ../backend_mcaquad/common/tinymt64.o
	g++ -o $@ $^ $(CFLAGS)	

clean:
	rm *.o test_main
