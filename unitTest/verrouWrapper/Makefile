export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

# Test of the verrou command (launcher/verrou)
.PHONY: run_check clean
all: run_check

prog: prog.c
	gcc -O1 -o $@ $<

run_check: prog
	./check.sh ./prog

clean:
	rm -rf prog out.* err.* fake_valgrind bin
