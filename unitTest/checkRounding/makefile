SOURCE=checkRounding.cxx
EXEC1=checkRounding
EXEC2=checkRoundingNative

export INSTALLPATH = $(shell bash ../installpath)
FLAGS += -I$(INSTALLPATH)/include/valgrind
FLAGS += -Wall -Wno-unused

FLAGS += -O1 #required to get hardware sqrt with builtin_sqrt

FMA_FLAG=
HAS_FMA = $(shell grep VERROU_COMPILED_WITH_FMA= $(INSTALLPATH)/env.sh)
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=yes)
	FMA_FLAG=-DTEST_FMA -DTEST_AVX
endif
ifeq ($(HAS_FMA), export VERROU_COMPILED_WITH_FMA=no)
	FMA_FLAG=-DTEST_NOFMA
endif

include ../flag.mk

FLAGS1 = $(FLAGS) $(DEBUGFLAGS) -DTEST_SSE
FLAGS2 = $(FLAGS) $(NATIVEFLAGS) -DTEST_SSE $(FMA_FLAG)

.PHONY: run run1 run2


run: run1 run2
run1: $(EXEC1)
	python3 runCheck.py $(EXEC1)

run2: $(EXEC2)
	python3 runCheck.py $(EXEC2)


$(EXEC1): $(SOURCE)
	$(CXX) $(FLAGS1) -o $(EXEC1) $(SOURCE)

$(EXEC2): $(SOURCE)
	$(CXX) $(FLAGS2) -o $(EXEC2) $(SOURCE)

clean:
	rm $(EXEC1) $(EXEC2)
