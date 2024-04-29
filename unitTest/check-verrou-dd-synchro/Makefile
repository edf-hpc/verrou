
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

FLAGS=-O0 -g -Wall -std=c++11 -DUSE_VERROU_SYNCHRO
VERROU_TASK_SO=-lverrouTask
CPP=g++

.PHONY:run

all:check
mainTime: mainTime.o 
	g++ $^  $(VERROU_TASK_SO) -o $@


%.o : %.cxx
	$(CPP) $(FLAGS) -c  $< -o $@


run: mainTime run.sh cmp.py
	$(INSTALLPATH)/bin/verrou_dd_synchro run.sh cmp.py


clean:
	rm -rf dd.synchro mainTime.o mainTime resDDsynchro

check: run
	cat dd.synchro/ddmin*/dd.synchro.include > resDDsynchro
	diff refNonReg resDDsynchro
