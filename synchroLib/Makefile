

FLAGS_VERROU=-I..

all: verrouSynchroLib.so verrouCBindingLib.so

verrouSynchroLib.so:verrouSynchroLib.o
	g++ -shared verrouSynchroLib.o -o verrouSynchroLib.so


verrouSynchroLib.o:verrouSynchroLib.cxx verrouSynchroLib.h
	g++ $(FLAGS_VERROU)  -Wall -Wextra  -c -fPIC verrouSynchroLib.cxx -o verrouSynchroLib.o


verrouCBindingLib.so:verrouCBindingLib.o
	gcc -shared verrouCBindingLib.o -o verrouCBindingLib.so


verrouCBindingLib.o:verrouCBinding.c verrouCBinding.h
	gcc $(FLAGS_VERROU)  -Wall -Wextra  -c -fPIC verrouCBinding.c -o verrouCBindingLib.o



clean:
	rm verrouSynchroLib.so verrouSynchroLib.o verrouCBindingLib.so verrouCBindingLib.o
