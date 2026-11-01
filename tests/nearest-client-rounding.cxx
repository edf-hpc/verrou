// The nearest and native rounding modes compute in nearest whatever the rounding mode of the client
#if defined(__x86_64__)
#include <fenv.h>
#include <stdio.h>
#include <immintrin.h>
#include "fma_common.hxx"

double one=1., small=1e-30, three=3., d0=0.1, d1=0.3, d2=0.7;
float one_f=1.f, small_f=1e-30f, d0_f=0.1f, d1_f=0.3f, d2_f=0.7f;

int main (int argc, char** argv) {
  fesetround (FE_DOWNWARD);
  double add=one+small;
  float add_f=one_f+small_f;
  double div=one/three;
  double madd=intrin_fma<double>(d0,d1,d2);
  float madd_f=intrin_fma<float>(d0_f,d1_f,d2_f);
  float cast=(float)d0;
  volatile long double x87=one, x87small=small;
  double x87add=(double)(x87-x87small);
  __m128d packed=_mm_add_pd(_mm_set1_pd(one), _mm_set1_pd(small));
  __m128 packed_f=_mm_add_ps(_mm_set1_ps(one_f), _mm_set1_ps(small_f));
  double sqrtRes=_mm_cvtsd_f64(_mm_sqrt_sd(_mm_set1_pd(three), _mm_set1_pd(three)));
  double sqrtPacked=_mm_cvtsd_f64(_mm_sqrt_pd(_mm_set1_pd(three)));
  fesetround (FE_TONEAREST);

  printf ("add double: %a\n", add);
  printf ("add float: %a\n", (double)add_f);
  printf ("div double: %a\n", div);
  printf ("madd double: %a\n", madd);
  printf ("madd float: %a\n", (double)madd_f);
  printf ("cast: %a\n", (double)cast);
  printf ("x87 sub: %a\n", x87add);
  printf ("packed double: %a\n", _mm_cvtsd_f64(packed));
  printf ("packed float: %a\n", (double)_mm_cvtss_f32(packed_f));
  printf ("sqrt scalar: %a\n", sqrtRes);
  printf ("sqrt packed: %a\n", sqrtPacked);
  return 0;
}
#else
int main (int argc, char** argv) {
  return 0;
}
#endif
