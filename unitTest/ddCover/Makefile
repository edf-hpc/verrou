
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash



.PHONY:

FLAGS=-Wall -g -O0


all: dd.line.post.assert dd.line.integer.post.assert dd.line.postmatch.assert dd.back.assert dd.back.line.assert diff.assert dd.back.line.integer.post.assert dd.back.line.post.assert dd.line.integer.vprec.assert

unitTest: unitTest.cxx integrate.hxx
	g++ $(FLAGS) -DFLOATLOOP -o $@ $<

unitTestInteger: unitTest.cxx integrate.hxx
	g++ $(FLAGS) -DINTEGERLOOP -o $@ $<

dd.line: unitTest ddRun.sh ddCmp.py
	verrou_dd_line --cache=clean --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRun.sh ./ddCmp.py

dd.line.integer: unitTestInteger ddRunInteger.sh ddCmp.py
	verrou_dd_line --cache=clean --rep=dd.line.integer --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRunInteger.sh ./ddCmp.py


dd.line.integer.vprec: unitTestInteger ddRunIntegerVprec.sh ddCmp.py dd.line.integer
	verrou_dd_line --cache=clean --rep=dd.line.integer.vprec --nruns=1 --num-threads=4 --seed=42 --rddmin-heuristics-rep=dd.line.integer  ./ddRunIntegerVprec.sh ./ddCmp.py


dd.line.integer.vprec.assert: dd.line.integer.vprec dd.line.integer
	diff dd.line.integer.vprec/ddmin0/dd.line.include  dd.line.integer/ddmin0/dd.line.include
	diff dd.line.integer.vprec/rddmin-cmp/dd.line.exclude  dd.line.integer/rddmin-cmp/dd.line.exclude
# exclude and not include because the search space is not the same due to --libm=instrumented

dd.back: unitTest ddRun.sh ddCmp.py
	verrou_dd_back --cache=clean --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRun.sh ./ddCmp.py

dd.back.line: unitTest ddRun.sh ddCmp.py dd.line
	VERROU_SOURCE=./dd.line/rddmin-cmp/dd.line.exclude verrou_dd_back --cache=clean --rep=dd.back.line --nruns=8 --res-with-all-samples --num-threads=4 --seed=42 ./ddRun.sh ./ddCmp.py

dd.back.line.integer: unitTest ddRunInteger.sh ddCmp.py dd.line.integer
	VERROU_SOURCE=./dd.line.integer/rddmin-cmp/dd.line.exclude verrou_dd_back --cache=clean --rep=dd.back.line.integer --nruns=8 --num-threads=4 --res-with-all-samples --seed=42 ./ddRunInteger.sh ./ddCmp.py


dd.back.line.post: dd.back.line
	cp -r dd.back.line dd.back.line.post
	post_verrou_dd --rep=dd.back.line.post --trace-bin --seed=42 --nrun=3 ./ddRun.sh ./ddCmp.py

dd.back.line.post.assert: dd.back.line.post
	./assertCover.py dd.back.line.post 1 8 3 0 default

dd.back.line.integer.post: dd.back.line.integer
	cp -r dd.back.line.integer dd.back.line.integer.post
	post_verrou_dd --rep=dd.back.line.integer.post --trace-bin --seed=42 --nrun=3 --rounding=sr_monotonic,nearest ./ddRunInteger.sh ./ddCmp.py

dd.back.line.integer.post.assert: dd.back.line.integer.post
	./assertCover.py dd.back.line.integer.post 1 8 3 0 sr_monotonic

dd.back.assert:dd.back
	./assertBack.py dd.back

dd.back.line.assert: dd.back.line
	./assertBack.py dd.back.line line

dd.line.post: dd.line
	cp -r dd.line dd.line.post
	post_verrou_dd --rep=dd.line.post --trace-bin --seed=42 --nrun=3 --rounding=average,nearest --count-denorm ./ddRun.sh ./ddCmp.py

dd.line.integer.post: dd.line.integer
	cp -r dd.line.integer dd.line.integer.post
	post_verrou_dd --rep=dd.line.integer.post --trace-bin --seed=42 --nrun=3 --rounding=average,nearest --count-denorm ./ddRunInteger.sh ./ddCmp.py

dd.line.integer.post.assert: dd.line.integer.post
	./assertCover.py dd.line.integer.post 1 8 3 0 average


dd.line.post.assert: dd.line.post
	./assertCover.py dd.line.post 2 8 3 0 average

dd.line.postmatch: dd.line
	cp -r dd.line dd.line.postmatch
	post_verrou_dd --nruns=4 --num-thread=4 --rep=dd.line.postmatch --trace-bin --seed=42 ./ddRunMatch.sh ./ddCmp.py

dd.line.postmatch.assert: dd.line.postmatch
	./assertCover.py dd.line.postmatch 2 8 4 2 default

diff.assert: dd.line.integer.post dd.line.post
	./assertDiffCover.sh

clean:
	rm -rf unitTest unitTestInteger dd.line dd.line.integer dd.line.integer.vprec dd.line.post dd.line.integer.post dd.line.postmatch matchLast.script.log-* *~
	rm -rf dd.back.line.integer.post dd.back.line.post dd.back.line.integer dd.back.line dd.back
