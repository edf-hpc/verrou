
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

FLAGS=-O0 -g -Wall -std=c++11 -DUSE_VERROU_TASK
VERROU_TASK_SO=-lverrouTask
CPP=g++

.PHONY:run

all:check
mainTime: mainTime.o 
	g++ $^  $(VERROU_TASK_SO) -o $@


%.o : %.cxx
	$(CPP) $(FLAGS) -c  $< -o $@


run: mainTime run.sh cmp.py
	$(INSTALLPATH)/bin/verrou_dd_task run.sh cmp.py


clean:
	rm -rf dd.task mainTime.o mainTime resDDtask

check: run
	cat dd.task/ddmin*/dd.task.include > resDDtask
	diff refNonReg resDDtask
