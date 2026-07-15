SOURCE=infiniteLoop.cxx
EXEC=infiniteLoop

export INSTALLPATH = $(shell bash ../installpath)
FLAGS += -Wall -Wno-unused
FLAGS += -O0 -g

VALGRIND_BIN=$(INSTALLPATH)/bin/valgrind
VERROU_CONTROL=$(INSTALLPATH)/bin/verrou_control

TRACE=trace.dat
TRACE_CONTROL=trace_control.dat

TRACE_CONTROL_FILTERED=trace_control_filtered.dat
TRACE_CONTROL_REF=trace_control_filtered.ref

.PHONY: run analyse

analyse: $(TRACE_CONTROL_FILTERED) $(TRACE_CONTROL_REF)
	diff  $(TRACE_CONTROL_REF) $(TRACE_CONTROL_FILTERED) 

$(TRACE_CONTROL_FILTERED):  $(TRACE_CONTROL)
	./pyFilter.py $(TRACE_CONTROL) > $(TRACE_CONTROL_FILTERED)


$(EXEC): $(SOURCE)
	$(CXX) $(FLAGS) -o $(EXEC) $(SOURCE)

$(TRACE) $(TRACE_CONTROL): $(EXEC)
	timeout 60 ${VALGRIND_BIN} --tool=verrou ./${EXEC} > trace.dat&
	sleep 1
	echo "${VERROU_CONTROL} --backtrace"          >  $(TRACE_CONTROL)
	${VERROU_CONTROL} --backtrace                 >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --verbose=off"        >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --verbose=off               >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --verbose=on"	      >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --verbose=on                >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --display-counter"    >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --display-counter           >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --hard-instr=stop"    >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --hard-instr=stop           >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --display-and-reset-counter" >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --display-and-reset-counter >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --display-counter"    >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --display-counter           >> $(TRACE_CONTROL)
	echo "${VERROU_CONTROL} --kill"               >> $(TRACE_CONTROL)
	${VERROU_CONTROL} --kill                      >> $(TRACE_CONTROL)




clean:
	rm -f $(EXEC) $(TRACE) $(TRACE_CONTROL) $(TRACE_CONTROL_FILTERED)
