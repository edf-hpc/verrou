export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash

# no definition before verrou.h, and warnings are errors: the header must be
# usable alone
FLAGS=-I$(INSTALLPATH)/include/valgrind -Wall -Wextra -Werror

.PHONY: run_check inventory clean

run_check: api api_cxx inventory
	./check.sh

# the inventory of the interfaces (docs/interfaces/inventory.md) must be
# generated from the current sources
inventory:
	python3 ../../docs/interfaces/gen_inventory.py --check

api: api.c
	$(CC) -std=c99 $(FLAGS) -o $@ $<

api_cxx: api_cxx.cxx
	$(CXX) $(FLAGS) -o $@ $<

clean:
	rm -f api api_cxx
