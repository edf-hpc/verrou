SOURCE=checkRounding.cxx
EXEC1=checkRounding
EXEC2=checkRoundingNative

VALGRINDPATH=$(shell ../../config.status --config | sed "s/.*--prefix=//" | sed "s/'//")
FLAGS +=-I$(VALGRINDPATH)/include/valgrind
FLAGS += -Wall -Wno-unused

FLAGS1 = $(FLAGS) -mfpmath=sse
FLAGS2 = $(FLAGS) -mfpmath=sse -march=native

.PHONY: run run1 run2

run: run1 run2
run1: $(EXEC1)
	python runCheck.py $(EXEC1)

run2: $(EXEC2)
	python runCheck.py $(EXEC2)


$(EXEC1): $(SOURCE)
	$(CXX) $(FLAGS1) -o $(EXEC1) $(SOURCE)

$(EXEC2): $(SOURCE)
	$(CXX) $(FLAGS2) -o $(EXEC2) $(SOURCE)
