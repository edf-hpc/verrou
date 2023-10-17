SRC=testAster.cxx
BIN=./testAster
FLAGS=-g -O0

export INSTALLPATH = $(shell bash ../installpath)
export VALGRIND=$(INSTALLPATH)/bin/valgrind --tool=verrou


REFERENCE=no
ifeq ($(REFERENCE), yes)
	MPFIINSTALL=/home/E52654/mpfi/mpfi-1.5.3/install
	FLAGS+= -I$(MPFIINSTALL)/include -DWITH_REFERENCE
	MPFILIB=$(MPFIINSTALL)/lib/libmpfi.a
	LDFLAGS= -lmpfr
endif

INTERLIBMREP=../../Interlibmath/
NUMSAMPLEIN=459 #probablility 0.99 and confidence level 1-alpha=0.99

data=resNative.dat resRandomverrou.dat resRandominterlibm.dat resRandomverrou+interlibm.dat reference.dat

.PHONY: $(data) $(INTERLIBMREP)/interlibmath.so
all: check


$(BIN): $(SRC)
	     g++ $(FLAGS) $(LDFLAGS) -o $@  $^ $(MPFILIB)

$(INTERLIBMREP)/interlibmath.so:
       make -C $(INTERLIBMREP) clean
       make -C $(INTERLIBMREP)

resNative.dat: $(BIN)
	$(BIN) > $@

#libm.ex: $(BIN)
#	ldd $(BIN)             |grep libm.so     |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" > libm.ex
#	ldd $(INTERLIBMREP)/interlibmath.so |grep libquadmath |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" >>libm.ex
#	ldd $(INTERLIBMREP)/interlibmath.so |grep libgcc_s    |cut -d " " -f 3 | xargs readlink -f | xargs echo "*" >libm.ex

resRandomverrou.dat: $(BIN)# libm.ex
	$(VALGRIND) --rounding-mode=random $(BIN) $(NUMSAMPLEIN) > $@

resRandominterlibm.dat: $(BIN) $(INTERLIBMREP)/interlibmath.so
	LD_PRELOAD=$(INTERLIBMREP)/interlibmath.so  VERROU_LIBM_ROUNDING_MODE=random  $(BIN) $(NUMSAMPLEIN) > $@

resRandomverrou+interlibm.dat: $(BIN)
	VERROU_ROUNDING_MODE=random $(VALGRIND) --libm=instrumented $(BIN) $(NUMSAMPLEIN) > $@

reference.dat:
	$(BIN) -1 > $@

tabAster.tex: ./genTab.py $(data)
	./genTab.py

testReg:tabAster.tex

check: testReg
	diff testReg testRegRef


clean:
	rm -f $(data) $(BIN) libm.ex tabAster.tex testReg

