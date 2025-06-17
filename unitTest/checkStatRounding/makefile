
SOURCE=checkStatRounding.cxx
EXEC=checkStatRounding

export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

#FLAGS += -I$(INSTALLPATH)/include/valgrind
FLAGS += -Wall -Wno-unused -O2 -g

FMA_FLAG=
HAS_FMA = $(shell grep VERROU_COMPILED_WITH_FMA= $(INSTALLPATH)/env.sh)
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=yes)
	FMA_FLAG=-DTEST_FMA
endif
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=no)
	FMA_FLAG=-DTEST_NOFMA
endif

include ../flag.mk

FLAGS1 = $(FLAGS) $(DEBUGFLAGS)


.PHONY: run cleanStat


test:$(EXEC) cleanStat run.sh extract.py
	ALGO_TYPE=float ALGO=Seq verrou_plot_stat --sample=2 --num-threads=2 --rounding=det,random,random_det,average,average_det --ref=upward --relative=104857.6 --png=floatSeq.png run.sh extract.py
	ALGO_TYPE=double ALGO=Seq verrou_plot_stat --rep=verrou.stat.2 --sample=2  --rounding=random  --no-plot run.sh extract.py
	ALGO_TYPE=double ALGO=Seq verrou_plot_stat --rep=verrou.stat.3 --seed=42 --sample=2  --rounding=random  --no-plot run.sh extract.py |grep -v verrou.stat.3 > res3
	ALGO_TYPE=double ALGO=Seq verrou_plot_stat --rep=verrou.stat.4 --seed=42 --sample=2  --rounding=random  --no-plot run.sh extract.py |grep -v verrou.stat.4 > res4
	diff res3 res4

$(EXEC): $(SOURCE)
	$(CXX) $(FLAGS1) -o $(EXEC) $(SOURCE)


clean: cleanStat
	rm -rf $(EXEC)

cleanStat:
	rm -rf verrou.stat verrou.stat.2 verrou.stat.3 verrou.stat.4
