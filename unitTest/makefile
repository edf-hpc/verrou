SUBREPLIST=checkRounding checkStatRounding checkUCB-vecto check-libM ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython

.PHONY: valgrind-test

all:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" || exit 42; \
	done


#to be able to do valgrind test from this directory
valgrind-test:
	make -C ../../tests check
	make -C ../../verrou check
	cd ../..; perl ./tests/vg_regtest verrou/tests


clean:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" clean ; \
	done
