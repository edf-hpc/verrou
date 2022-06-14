SOURCE=checkStatRounding.cxx
EXEC=checkStatRounding

export INSTALLPATH = $(shell bash ../installpath)
#FLAGS += -I$(INSTALLPATH)/include/valgrind
FLAGS += -Wall -Wno-unused

FMA_FLAG=
HAS_FMA = $(shell grep VERROU_COMPILED_WITH_FMA= $(INSTALLPATH)/env.sh)
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=yes)
	FMA_FLAG=-DTEST_FMA
endif
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=no)
	FMA_FLAG=-DTEST_NOFMA
endif

include ../flag.mk

FLAGS = $(FLAGS) $(DEBUGFLAGS)


.PHONY: run 

run: $(EXEC) run.sh extract.py
	ALGO_TYPE=float ALGO=Seq verrou_plot_stat run.sh extract.py

$(EXEC): $(SOURCE)
	$(CXX) $(FLAGS1) -o $(EXEC) $(SOURCE)


clean:
	rm -rf $(EXEC) verrou.stat
