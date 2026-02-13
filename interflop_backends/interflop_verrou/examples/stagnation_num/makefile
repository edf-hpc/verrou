.PHONY:
TINYREP=../../../backend_mcaquad/common/

SRC=stagnation_nearness.cxx ../../interflop_verrou.cxx $(TINYREP)/tinymt64.c

CFLAGS_GEN=-I ../.. -I../ -O2 -g -Wall -march=native -DVERROU_DET_HASH=vr_xxhash_hash
CFLAGS=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=1
CFLAGS_1=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=1
CFLAGS_2=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=2
CFLAGS_3=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=3
CFLAGS_4=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=4
CFLAGS_8=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=8
CFLAGS_16=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=16
CFLAGS_32=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=32
CFLAGS_64=$(CFLAGS_GEN) -DVERROU_NUM_NEARNESS=64

nearness_study: nearness1_stagnation nearness2_stagnation nearness3_stagnation nearness4_stagnation nearness8_stagnation nearness16_stagnation nearness32_stagnation nearness64_stagnation 
	./nearness1_stagnation
	./nearness2_stagnation
	./nearness3_stagnation
	./nearness4_stagnation
	./nearness8_stagnation
	./nearness16_stagnation
	./nearness32_stagnation
	./nearness64_stagnation
	./postTreatmentNearness.py
	gnuplot nearnessStagnationLog.gp

nearness1_stagnation: $(SRC)
	g++ $(CFLAGS_1) -o $@ $(SRC)

nearness2_stagnation: $(SRC)
	g++ $(CFLAGS_2) -o $@ $(SRC)

nearness3_stagnation: $(SRC)
	g++ $(CFLAGS_3) -o $@ $(SRC)

nearness4_stagnation: $(SRC)
	g++ $(CFLAGS_4) -o $@ $(SRC)

nearness8_stagnation: $(SRC)
	g++ $(CFLAGS_8) -o $@ $(SRC)

nearness16_stagnation: $(SRC)
	g++ $(CFLAGS_16) -o $@ $(SRC)

nearness32_stagnation: $(SRC)
	g++ $(CFLAGS_32) -o $@ $(SRC)

nearness64_stagnation: $(SRC)
	g++ $(CFLAGS_64) -o $@ $(SRC)



clean:
	rm -f  nearness*_stagnation *.out
