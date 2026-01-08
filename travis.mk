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

reconfigure-nosqrt:
	@echo "*** CONFIGURE NO SQRT***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --enable-verrou-sqrt=no --prefix=$${PWD}/../install

reconfigure-nofma:
	@echo "*** CONFIGURE NO FMA***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --enable-verrou-fma=no --prefix=$${PWD}/../install

reconfigure-nochecknaninf:
	@echo "*** CONFIGURE NO NAN INF CHECK***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --enable-verrou-check-naninf=no --prefix=$${PWD}/../install

reconfigure-allhack:
	@echo "*** CONFIGURE DENORM HACK ALL***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --with-verrou-denorm-hack=all --prefix=$${PWD}/../install

reconfigure-nohack:
	@echo "*** CONFIGURE DENORM HACK NO***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --with-verrou-denorm-hack=none --prefix=$${PWD}/../install

reconfigure-noxoshiro:
	@echo "*** CONFIGURE XOSHIRO  NO***"
	cd ../valgrind+verrou && ./configure --enable-only64bit  --enable-verrou-xoshiro=no  --prefix=$${PWD}/../install

reconfigure-noquadmath:
	@echo "*** CONFIGURE QUAD NO ***"
	cd ../valgrind+verrou && ./configure --enable-only64bit  --enable-verrou-quadmath=no  --prefix=$${PWD}/../install

reconfigure-AVG4:
	@echo "*** CONFIGURE VERROU_NUM_AVG=4 ***"
	cd ../valgrind+verrou && VERROU_NUM_AVG=4 ./configure --enable-only64bit --prefix=$${PWD}/../install


reconfigure-disableoptim:
	@echo "*** CONFIGURE --disable-verrou-optim ***"
	cd ../valgrind+verrou && ./configure --enable-only64bit --disable-verrou-optim --prefix=$${PWD}/../install



clean:
	@echo "*** MAKE clean ***"
	cd ../valgrind+verrou/verrou && make clean


build:
	@echo "*** MAKE ***"
	cd ../valgrind+verrou && make -j4

	@echo "*** MAKE INSTALL ***"
	cd ../valgrind+verrou && make install

check-install:
	@echo "*** CHECK VERSION ***"
	source ../install/env.sh && valgrind --version

	@echo "*** CHECK HELP ***"
	source ../install/env.sh && valgrind --tool=verrou --help

check:
	@echo "*** BUILD TESTS ***"
	cd ../valgrind+verrou/ && make check && ./tests/vg_regtest none

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
