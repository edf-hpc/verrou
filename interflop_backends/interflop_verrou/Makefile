SRC_BACKEND=interflop_verrou.cxx

SRC=test_main.cxx

CFLAGS=-g -Wall -march=native


%.o: %.cxx
	g++ -o $@ -c $< $(CFLAGS)


test_main: test_main.o interflop_verrou.o ../backend_mcaquad/common/tinymt64.o
	g++ -o $@ $^ $(CFLAGS)	

clean:
	rm *.o test_main
