SUBREPLIST=ddCover checkRounding checkStatRounding checkUCB-vecto check-libM ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython checkVPREC verrou_control

SUBREPLISTPYTHON=ddCover checkStatRounding ddTest check-verrou-dd-task ddStdOut ddLibm verrouDDPython

.PHONY: valgrind-test none-test callgrind-test verrou-test python-coverage

all:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" -j1|| exit 42; \
	done

python:
	for i in $(SUBREPLISTPYTHON) ; do \
		make -C "./$$i" || exit 42; \
	done

# Coverage of the installed python tools by the 'python' tests
# (requires python3-coverage). The report is written in pyCoverageResult.
# The test directories are cleaned first: up-to-date results would not run the tools.
PYCOV_OUT=$(CURDIR)/pyCoverageResult

python-coverage:
	for i in $(SUBREPLISTPYTHON) ; do \
		make -C "./$$i" clean ; \
	done
	rm -rf "$(PYCOV_OUT)" && mkdir -p "$(PYCOV_OUT)"
	export VERROU_PYCOV_PREFIX="$$(cd ddCover && bash ../installpath)" && \
	export VERROU_PYCOV_PKGDIR="$$(bash -c 'source ../env.sh; python3 -c "import DD,os; print(os.path.dirname(os.path.abspath(DD.__file__)))"')" && \
	export VERROU_PYCOV_OUT="$(PYCOV_OUT)" VERROU_PYCOV_SRC="$(CURDIR)/.." && \
	export COVERAGE_PROCESS_START="$(CURDIR)/pyCoverage/coveragerc" && \
	export PYTHONPATH="$(CURDIR)/pyCoverage$${PYTHONPATH:+:$$PYTHONPATH}" && \
	{ $(MAKE) python ; status=$$? ; \
	  unset PYTHONPATH ; \
	  python3 -m coverage combine -q --rcfile="$$COVERAGE_PROCESS_START" && \
	  python3 -m coverage html -q --rcfile="$$COVERAGE_PROCESS_START" && \
	  python3 -m coverage report --rcfile="$$COVERAGE_PROCESS_START" | tee "$(PYCOV_OUT)/report.txt" ; \
	  exit $$status ; }

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
	rm ../../none/tests/replaces_malloc_client_req.vgtest  #comment this testcase. This test depends on the tool name
	cd ../.. ; perl ./tests/vg_regtest none/tests --force-tool=verrou --stderr-ignore
	cd ../.. ; git checkout -- none/tests/linux/getdents_filter.vgtest none/tests/replaces_malloc_client_req.vgtest #recover file to pass post-regtest-checks

callgrind-test: valgrind-check
	cd ../..; perl ./tests/vg_regtest callgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore --cleanup-ignore --post-ignore

helgrind-test: valgrind-check
	make -C ../../helgrind check
	cd ../..; perl ./tests/vg_regtest helgrind/tests --force-tool=verrou --stderr-ignore --toolopt-ignore

clean:
	for i in $(SUBREPLIST) ; do \
		make -C "./$$i" clean ; \
	done
