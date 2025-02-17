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
	cd ../valgrind+verrou && ./configure --enable-only64bit --prefix=$${PWD}/../install

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
	make -C ../valgrind+verrou/verrou/unitTest/ valgrind-test

check-error:
	../valgrind+verrou/verrou/tests/post_diff.sh ../valgrind+verrou/none/tests/
	../valgrind+verrou/verrou/tests/post_diff.sh ../valgrind+verrou/callgrind/tests/
	../valgrind+verrou/verrou/tests/post_verrou_diff.sh ../valgrind+verrou/verrou/tests/
	@false

unit-test:
	@echo "*** VERROU UNIT TESTS ***"
	cd ../valgrind+verrou/verrou/unitTest && make

valgrind-test:
	@echo "*** VALGRIND UNIT TESTS ***"
	cd ../valgrind+verrou/verrou/unitTest && make valgrind-test

post-regtest-checks:
	@echo "*** POST_REGTEST_CHECKS ***"
	cd ../valgrind+verrou/ && make post-regtest-checks

gitignore-checks:
	@echo "*** GITIGNORE_CHECKS ***"
	cd ../valgrind+verrou/verrou && git status --porcelain | grep '^??' | cut -c4- |tee /tmp/tracked_file.tmp
	cat /tmp/tracked_file.tmp | wc -l |xargs test 0 -eq
