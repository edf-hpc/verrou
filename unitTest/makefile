.PHONY: checkRounding checkUCB-vecto valgrind-test


all: checkRounding checkUCB-vecto
checkRounding:
	make -C ./checkRounding

checkUCB-vecto:
	make -C ./checkUCB-vecto


#to be able to do valgrind test from this directory
valgrind-test:
	make -C ../../tests check
	make -C ../../verrou check
	cd ../..; perl ./tests/vg_regtest verrou
