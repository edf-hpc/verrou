
INCLUDE=-I.. -I../../..

SRC= ../interflop_vprec.c ../common/vprec_tools.c ../../../interflop/interflop_stdlib.c ../../../interflop/iostream/logger.c ../../../interflop/sqrt/interflop_sqrt.c ../../../interflop/fma/interflop_fma.c 


FLAGS= -Wall -Wextra -Wno-varargs -O0 -g -fno-builtin -DIGNORE_VFI_CONTEXT -msse2
LDFLAGS=-lm -lmpfr

all: debugCheckVPREC cmpFloat infCheck smallExpo

debugCheckVPREC: debugCheckVPREC.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) debugCheckVPREC.c $(SRC) -o debugCheckVPREC $(LDFLAGS)


cmpFloat: cmpFloat.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) cmpFloat.c $(SRC) -o cmpFloat $(LDFLAGS)

smallExpo: smallExpo.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) smallExpo.c $(SRC) -o smallExpo $(LDFLAGS)


infCheck: infCheck.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) infCheck.c $(SRC) -o infCheck $(LDFLAGS)

clean:
	rm infCheck cmpFloat
