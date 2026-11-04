/* sqrt is only counted with mcaquad : a block with sqrt as only floating-point
   operation is not instrumented, also with the backtrace options */
#include <stdio.h>
#if defined(__x86_64__)
#include <emmintrin.h>
#endif

int main (int argc, char** argv) {
#if defined(__x86_64__)
  volatile double a=2.;
  __m128d res=_mm_sqrt_pd (_mm_set1_pd (a));
  printf ("%f\n", _mm_cvtsd_f64 (res));
#endif
  return 0;
}
