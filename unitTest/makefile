SUBREPLIST=checkRounding checkStatRounding checkUCB-vecto check-libM ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython

.PHONY: valgrind-test none-test callgrind-test verrou-test

all:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" || exit 42; \
	done


#to be able to do valgrind test from this directory
valgrind-test: verrou-test none-test callgrind-test #helgrind-test

verrou-test:
	make -C ../../tests check
	make -C ../../verrou check
	cd ../..; perl ./tests/vg_regtest verrou/tests

none-test:
	make -C ../../tests check
	cd ../..; perl ./tests/vg_regtest none/tests --force-tool=verrou --stderr-ignore

callgrind-test:
	make -C ../../tests check
	cd ../..; perl ./tests/vg_regtest callgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore --cleanup-ignore --post-ignore

helgrind-test:
	make -C ../../tests check
	cd ../..; perl ./tests/vg_regtest helgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore

clean:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" clean ; \
	done
