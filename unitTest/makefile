.PHONY: checkRounding checkUCB-vecto check-libM check-dd check-verrou-dd-synchro valgrind-test


all: checkRounding checkUCB-vecto check-libM check-dd check-verrou-dd-synchro
checkRounding:
	make -C ./checkRounding

checkUCB-vecto:
	make -C ./checkUCB-vecto

check-libM:
	make -C ./check-libM

check-verrou-dd-synchro:
	make -C ./check-verrou-dd-synchro

check-dd:
	make -C ./ddTest

#to be able to do valgrind test from this directory
valgrind-test:
	make -C ../../tests check
	make -C ../../verrou check
	cd ../..; perl ./tests/vg_regtest verrou/tests
