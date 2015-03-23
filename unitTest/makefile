

SOURCE=checkRounding.cxx
EXEC1=checkRoundingNative
EXEC2=checkRounding387

SOURCECOS=checkCosSin.cxx
COS1=checkCosNative

SOURCEADD=addVec.cxx
ADDVEC=addVec

VALGRINDPATH=$(shell ../../config.status --config | sed "s/.*--prefix=//" | sed "s/'//")
FLAGS +=-I$(VALGRINDPATH)/include/valgrind
FLAGS += -Wall -Wno-unused

FLAGS1=$(FLAGS) #-march=native -DTEST_FMA
FLAGS2=$(FLAGS) -mfpmath=387  

.PHONY: run run1 run2 runvalcos

run: run1 run2
run1: $(EXEC1)
	python runCheck.py $(EXEC1)

run2: $(EXEC2)
	python runCheck.py $(EXEC2)


$(EXEC1): $(SOURCE) # $(wildcard ../*.h)
	$(CXX) $(FLAGS1) -o $(EXEC1) $(SOURCE)
$(EXEC2): $(SOURCE) # $(wildcard ../*.h)
	$(CXX) $(FLAGS2) -o $(EXEC2) $(SOURCE)

$(COS1): $(SOURCECOS) makefile # $(wildcard ../*.h)
#	$(CXX) $(FLAGS) -lm -O3 -fno-builtin -g -o $(COS1) $(SOURCECOS)
	$(CXX) $(FLAGS) -O3  -o $(COS1) $(SOURCECOS)
runvalcos:$(COS1) makefile
	$(VALGRINDPATH)/bin/valgrind --tool=verrou --count-op=yes --rounding-mode=random --verrou-verbose=no ./$(COS1)

$(ADDVEC): $(SOURCEADD) # $(wildcard ../*.h)
	$(CXX) $(FLAGS) -O3 -o $(ADDVEC) $(SOURCEADD)

runAddV:$(ADDVEC) makefile
	$(VALGRINDPATH)/bin/valgrind --tool=verrou --count-op=yes --rounding-mode=random --verrou-verbose=yes ./$(ADDVEC) 0.1 0.01 0.2 0.01
	$(VALGRINDPATH)/bin/valgrind --tool=verrou --count-op=yes --rounding-mode=random --verrou-verbose=yes ./$(ADDVEC) 0.1 0.2 0.2 0.1


