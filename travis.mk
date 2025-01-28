SHELL = /bin/bash

download-valgrind:
	cd .. && git clone --branch=$(VALGRIND_VERSION) --single-branch git://sourceware.org/git/valgrind.git valgrind+verrou >/dev/null

patch-valgrind:
	cd ../valgrind+verrou && cp -a $(PWD) verrou
	cd ../valgrind+verrou && cat verrou/valgrind.*diff | patch -p1

patch-error:
	cd ../valgrind+verrou && find . -name '*.rej' | xargs tail -n+1
	# try to build verrou anyway if we check the development version of Valgrind
	test "$(VALGRIND_VERSION)" = "master"

configure:
	@echo "*** AUTOGEN ***"
	cd ../valgrind+verrou && ./autogen.sh

	@echo "*** CONFIGURE ***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --enable-verrou-fma=yes --prefix=$${PWD}/../install

build:
	@echo "*** MAKE ***"
	cd ../valgrind+verrou && make

	@echo "*** MAKE INSTALL ***"
	cd ../valgrind+verrou && make install

check-install:
	@echo "*** CHECK VERSION ***"
	source ../install/env.sh && valgrind --version

	@echo "*** CHECK HELP ***"
	source ../install/env.sh && valgrind --tool=verrou --help

check:
	@echo "*** BUILD TESTS ***"
	cd ../valgrind+verrou && make -C tests  check
	cd ../valgrind+verrou && make -C verrou check

	@echo "*** VALGRIND TESTS ***"
	cd ../valgrind+verrou && perl tests/vg_regtest verrou

check-error:
	cd ../valgrind+verrou/verrou/tests && tail -n+1 *.stdout.diff *.stdout.out *.stderr.diff *.stderr.out
	@false

unit-test:
	@echo "*** UNIT TESTS ***"
	cd ../valgrind+verrou/verrou/unitTest && make

post-regtest-checks:
	@echo "*** POST_REGTEST_CHECKS ***"
	cd ../valgrind+verrou/ && make post-regtest-checks

gitignore-checks:
	@echo "*** GITIGNORE_CHECKS ***"
	cd ../valgrind+verrou/verrou && git status --porcelain | grep '^??' | cut -c4- |tee /tmp/tracked_file.tmp
	cat /tmp/tracked_file.tmp | wc -l |xargs test 0 -eq
