



SOURCE=checkRounding.cxx
EXEC=checkRounding

VALGRINDPATH=/home/E52654/codeVerrou/install/
VALGRINDBIN=$(VALGRINDPATH)/bin/valgrind
VALGRINDCMD=$(VALGRINDBIN) --tool=verrou --verrou-verbose=no --rounding-mode=
FLAGS+=-I$(VALGRINDPATH)include/valgrind
#FLAGS += -I../ -DIGNOREVALGRIND 
FLAGS += -O0 -g -mfpmath=sse -msse2 -Wall -Wno-unused

DEVNULL= #2>/dev/null

.PHONY: runFenv runValgrind run
run: runFenv runValgrind

runFenv: $(EXEC) makefile
	./$(EXEC) NEAREST FENV
	./$(EXEC) TOWARDZERO FENV
	./$(EXEC) DOWNWARD FENV
	./$(EXEC) UPWARD FENV
	@echo ""

runValgrind: $(EXEC) makefile
	$(VALGRINDCMD)nearest ./$(EXEC) NEAREST valgrind  $(DEVNULL)
	$(VALGRINDCMD)toward_zero ./$(EXEC) TOWARDZERO valgrind  $(DEVNULL)
	$(VALGRINDCMD)downward ./$(EXEC) DOWNWARD valgrind  $(DEVNULL)
	$(VALGRINDCMD)upward ./$(EXEC) UPWARD valgrind  $(DEVNULL)
	@echo "only Val"
	$(VALGRINDCMD)average ./$(EXEC) UPWARD valgrind  $(DEVNULL)
	$(VALGRINDCMD)random ./$(EXEC) UPWARD valgrind  $(DEVNULL)

$(EXEC): $(SOURCE) $(wildcard ../*.h)
	$(CXX) $(FLAGS) -o $(EXEC) $(SOURCE)

