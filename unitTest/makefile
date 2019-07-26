.PHONY: checkRounding checkUCB-vecto check-libM valgrind-test


all: checkRounding checkUCB-vecto check-libM
checkRounding:
	make -C ./checkRounding

checkUCB-vecto:
	make -C ./checkUCB-vecto

check-libM:
	make -C ./check-libM
#to be able to do valgrind test from this directory
valgrind-test:
	make -C ../../tests check
	make -C ../../verrou check
	cd ../..; perl ./tests/vg_regtest verrou/tests
