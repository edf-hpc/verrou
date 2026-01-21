
INCLUDE=-I.. -I../../..

SRC= ../interflop_vprec.c ../common/vprec_tools.c ../../../interflop/interflop_stdlib.c ../../../interflop/iostream/logger.c ../../../interflop/sqrt/interflop_sqrt.c ../../../interflop/fma/interflop_fma.c 


FLAGS= -Wall -Wextra -Wno-varargs -O2 -fno-builtin -DIGNORE_VFI_CONTEXT


infCheck: infCheck.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) infCheck.c $(SRC) -o infCheck -lm

basic: basic.c $(SRC)
	gcc $(FLAGS) $(INCLUDE) basic.c $(SRC) -o basic -lm

clean:
	rm infCheck basic
