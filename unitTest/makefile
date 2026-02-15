SUBREPLIST=checkRounding checkStatRounding checkUCB-vecto check-libM ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython ddCover checkVPREC

SUBREPLISTPYTHON=checkStatRounding ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython ddCover

.PHONY: valgrind-test none-test callgrind-test verrou-test

all:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" || exit 42; \
	done

python:
	for i in $(SUBREPLISTPYTHON) ; do \
		make -C "./$$i" || exit 42; \
	done


#to be able to do valgrind test from this directory
valgrind-test: verrou-test none-test callgrind-test #helgrind-test

valgrind-check:
	make -C ../.. check
	make -C ../../tests check
	make -C ../../verrou check
	make -C ../../none check
	make -C ../../perf check
	make -C ../../callgrind check

verrou-test: valgrind-check
	cd ../..; perl ./tests/vg_regtest verrou/tests

none-test: valgrind-check
	rm ../../none/tests/linux/getdents_filter.vgtest #comment this testcase. Already fail with none tool test (bug with hyperv suspected)
	cd ../.. ; perl ./tests/vg_regtest none/tests --force-tool=verrou --stderr-ignore
	cd ../.. ; git checkout -- none/tests/linux/getdents_filter.vgtest # recover file to pass post-regtest-checks

callgrind-test: valgrind-check
	cd ../..; perl ./tests/vg_regtest callgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore --cleanup-ignore --post-ignore

helgrind-test: valgrind-check
	make -C ../../helgrind check
	cd ../..; perl ./tests/vg_regtest helgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore

clean:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" clean ; \
	done
