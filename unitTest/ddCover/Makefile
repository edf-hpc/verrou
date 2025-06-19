
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash



.PHONY:

FLAGS=-Wall -g -O0


all: dd.line.post.assert dd.line.integer.post.assert dd.line.postmatch.assert dd.back.assert dd.back.line.assert diff.assert

unitTest: unitTest.cxx integrate.hxx
	g++ $(FLAGS) -DFLOATLOOP -o $@ $<

unitTestInteger: unitTest.cxx integrate.hxx
	g++ $(FLAGS) -DINTEGERLOOP -o $@ $<

dd.line: unitTest ddRun.sh ddCmp.py
	verrou_dd_line --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRun.sh ./ddCmp.py
	touch dd.line #to ensure dependency

dd.line.integer: unitTestInteger ddRun.sh ddCmp.py
	verrou_dd_line --rep=dd.line.integer --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRunInteger.sh ./ddCmp.py
	touch dd.line.integer #to ensure dependency

dd.back: unitTest ddRun.sh ddCmp.py
	verrou_dd_back --nruns=8 --num-threads=4 --seed=42 ./ddRun.sh ./ddCmp.py
	touch dd.back #to ensure dependency

dd.back.line: unitTest ddRun.sh ddCmp.py dd.line
	VERROU_SOURCE=./dd.line/rddmin-cmp/dd.line.exclude verrou_dd_back --rep=dd.back.line --nruns=8 --num-threads=4 --seed=42 ./ddRun.sh ./ddCmp.py
	touch dd.back.line #to ensure dependency

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
	rm -rf unitTest dd.line dd.line.integer dd.line.post dd.line.integer.post dd.line.postmatch matchLast.script.log-* *~ dd.back dd.back.line
