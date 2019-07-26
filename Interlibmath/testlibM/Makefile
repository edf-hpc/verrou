SRC=testAster.cxx
BIN=./testAster
FLAGS=-g -O0

REFERENCE=no
ifeq ($(REFERENCE), yes)
	MPFIINSTALL=/home/E52654/mpfi/mpfi-1.5.3/install
	FLAGS+= -I$(MPFIINSTALL)/include -DWITH_REFERENCE
	MPFILIB=$(MPFIINSTALL)/lib/libmpfi.a
	LDFLAGS= -lmpfr
endif

INTERLIBMREP=../Interlibmath/
NUMSAMPLEIN=1000
NUMSAMPLEOUT=1

.PHONY: dat
all: tabAster.tex

data=resNative.dat resRandomverrou.dat resRandominterlibm.dat resRandomverrou+interlibm.dat reference.dat

../interlibmath.so:
	make -C $(INTERLIBMREP)

$(BIN): $(SRC)
	     g++ $(FLAGS) $(LDFLAGS) -o $@  $^ $(MPFILIB)

resNative.dat: $(BIN)
	$(BIN) > $@

libm.ex: $(BIN) ../interlibmath.so
	ldd $(BIN)             |grep libm.so     |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" > libm.ex
	ldd ../interlibmath.so |grep libquadmath |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" >>libm.ex
	ldd ../interlibmath.so |grep libgcc_s    |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" >>libm.ex

resRandomverrou.dat: $(BIN) libm.ex
	valgrind --demangle=no --tool=verrou --rounding-mode=random --exclude=libm.ex $(BIN) $(NUMSAMPLEIN) > $@

resRandominterlibm.dat: $(BIN) ../interlibmath.so
	LD_PRELOAD=../interlibmath.so VERROU_ROUNDING_MODE=random $(BIN) $(NUMSAMPLEIN) > $@

resRandomverrou+interlibm.dat: $(BIN) ../interlibmath.so libm.ex
	LD_PRELOAD=../interlibmath.so VERROU_ROUNDING_MODE=random valgrind --demangle=no --tool=verrou --exclude=libm.ex $(BIN) $(NUMSAMPLEIN) > $@

reference.dat:
	$(BIN) -1 > $@

tabAster.tex: ./genTab.py $(data)
	./genTab.py

testReg:tabAster.tex

check: testReg
	diff testReg testRegRef


clean:
	rm -f $(data) $(BIN) libm.ex tabAster.tex testReg

