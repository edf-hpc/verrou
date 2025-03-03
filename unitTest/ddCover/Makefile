
export INSTALLPATH = $(shell bash ../installpath)
export BASH_ENV=$(INSTALLPATH)/env.sh
export SHELL=bash



.PHONY:

FLAGS=-Wall -g -O2


all: dd.line.post.assert dd.line.postmatch.assert

unitTest: unitTest.cxx integrate.hxx
	g++ $(FLAGS) -o $@ $<


dd.line: unitTest ddRun.sh ddCmp.py
	verrou_dd_line --nruns=8 --num-threads=4 --seed=42 --res-with-all-samples ./ddRun.sh ./ddCmp.py
	touch dd.line #to ensure dependency

dd.line.post: dd.line
	cp -r dd.line dd.line.post
	post_verrou_dd --rep=dd.line.post --trace-bin --seed=42 --nrun=3 --rounding=average,nearest --count-denorm ./ddRun.sh ./ddCmp.py

dd.line.post.assert: dd.line.post
	./assertCover.py dd.line.post 2 8 3 0 average

dd.line.postmatch: dd.line
	cp -r dd.line dd.line.postmatch
	post_verrou_dd --nruns=4 --num-thread=4 --rep=dd.line.postmatch --trace-bin --seed=42 ./ddRunMatch.sh ./ddCmp.py

dd.line.postmatch.assert: dd.line.postmatch
	./assertCover.py dd.line.postmatch 2 8 4 2 default


clean:
	rm -rf unitTest dd.line dd.line.post dd.line.postmatch matchLast.script.log-* *~
