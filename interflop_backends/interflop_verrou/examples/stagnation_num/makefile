.PHONY:
TINYREP=../../../backend_mcaquad/common/

SRC=stagnation_avg.cxx ../../interflop_verrou.cxx $(TINYREP)/tinymt64.c

CFLAGS_GEN=-I ../.. -I../ -O2 -g -Wall -march=native -DVERROU_DET_HASH=vr_xxhash_hash
CFLAGS=$(CFLAGS_GEN) -DVERROU_NUM_AVG=1
CFLAGS_1=$(CFLAGS_GEN) -DVERROU_NUM_AVG=1
CFLAGS_2=$(CFLAGS_GEN) -DVERROU_NUM_AVG=2
CFLAGS_3=$(CFLAGS_GEN) -DVERROU_NUM_AVG=3
CFLAGS_4=$(CFLAGS_GEN) -DVERROU_NUM_AVG=4
CFLAGS_8=$(CFLAGS_GEN) -DVERROU_NUM_AVG=8
CFLAGS_16=$(CFLAGS_GEN) -DVERROU_NUM_AVG=16
CFLAGS_32=$(CFLAGS_GEN) -DVERROU_NUM_AVG=32
CFLAGS_64=$(CFLAGS_GEN) -DVERROU_NUM_AVG=64

avg_study: avg1_stagnation avg2_stagnation avg3_stagnation avg4_stagnation avg8_stagnation avg16_stagnation avg32_stagnation avg64_stagnation 
	./avg1_stagnation
	./avg2_stagnation
	./avg3_stagnation
	./avg4_stagnation
	./avg8_stagnation
	./avg16_stagnation
	./avg32_stagnation
	./avg64_stagnation
	./postTreatmentAvg.py
	gnuplot avgStagnationLog.gp

avg1_stagnation: $(SRC)
	g++ $(CFLAGS_1) -o $@ $(SRC)

avg2_stagnation: $(SRC)
	g++ $(CFLAGS_2) -o $@ $(SRC)

avg3_stagnation: $(SRC)
	g++ $(CFLAGS_3) -o $@ $(SRC)

avg4_stagnation: $(SRC)
	g++ $(CFLAGS_4) -o $@ $(SRC)

avg8_stagnation: $(SRC)
	g++ $(CFLAGS_8) -o $@ $(SRC)

avg16_stagnation: $(SRC)
	g++ $(CFLAGS_16) -o $@ $(SRC)

avg32_stagnation: $(SRC)
	g++ $(CFLAGS_32) -o $@ $(SRC)

avg64_stagnation: $(SRC)
	g++ $(CFLAGS_64) -o $@ $(SRC)



clean:
	rm -f  avg*_stagnation *.out
