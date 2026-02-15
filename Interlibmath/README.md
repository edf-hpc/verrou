# Interlibmath
Interlibmath is a library to intercept (with LD_PRELOAD) libm calls and replace it by the equivalent calls with the specified rounding mode. It helps you look for floating-point round-off errors in programs introduced by the libm.


## Disclamer
1- not yet sufficiently tested
2- interface is not yet stabilized
3- a lot of libm function are missing
4- performance not yet optimized
5- it relies on libquadmath to provide round to nearest


## Compilation :
Not yet included in verrou/valgrind compilation process
make will produce the file interlibmath.so

## How to use in standalone
VERROU_LIBM_ROUNDING_MODE=random LD_PRELOAD='PATH'/interlibmath.so. /myProg
or 
VERROU_ROUNDING_MODE=random LD_PRELOAD='PATH'/interlibmath.so. /myProg

If both  VERROU_LIBM_ROUNDING_MODE and VERROU_ROUNDING_MODE are set, interlibmath take into account VERROU_LIBM_ROUNDING_MODE.

These variable can be to set to the values : random, nearness, nearest, upward, downward,toward_zero,farthest,float,native

The meaning is the same for Verrou.
The difference between native and nearest is that : native return the value obtain by native implementation and nearest return the nearest cast of the result obtain by libquadmath.



## How to use with Verrou :

VERROU_ROUNDING_MODE=random LD_PRELOAD='PATH'/interlibmath.so valgrind --tool=verrou --exclude=libm.ex ./myProg
Remarks :
- libm.ex should contain the libm and libquadmath.
- libm functions and ./myProg operation are instrumented with random rounding

To set different rounding mode to libm (random)  and to myProg (upward):
   VERROU_LIBM_ROUNDING_MODE=random VERROU_ROUNDING_MODE=upward  LD_PRELOAD='PATH'/interlibmath.so valgrind --tool=verrou --exclude=libm.ex ./myProg	
or
   VERROU_LIBM_ROUNDING_MODE=random LD_PRELOAD='PATH'/interlibmath.so valgrind --tool=verrou --exclude=libm.ex --rounding-mode=upward ./myProg		


## Remarks

If your favorite libm function does not appear in counters :
   1- interlibm do not implement it => mail to developers (it is easy to add if the function has only one argument)
   2- your function may be inlined by compiler => modify your compiler option
