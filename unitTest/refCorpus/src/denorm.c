/* R1 reference corpus: denormal numbers (checkdenorm backend:
 * --check-denorm, --count-denorm, --cdi-gen-file, --cdo-gen-file, and the
 * daz/ftz rounding modes). Operations produce or read denormals, in scalar
 * and packed forms; the denormal counters are printed and reset by client
 * requests between two phases. Results are printed exactly (%a).
 * Build: -O1 -g -mavx2 -mfma -ffp-contract=off */
#include <float.h>
#include <immintrin.h>
#include <stdio.h>
typedef int vr_RoundingMode;
typedef unsigned char Bool;
#include "verrou.h"

static volatile double dmin = DBL_MIN, three = 3.0, half = 0.5, den = DBL_MIN / 4.0;
static volatile float fmin_ = FLT_MIN;

__attribute__((noinline)) static double out_div(void) { return dmin / three; }
__attribute__((noinline)) static double out_mul(void) { return dmin * half; }
__attribute__((noinline)) static double out_sub(void) { return dmin * 1.5 - dmin; }
__attribute__((noinline)) static double in_add(void) { return den + den; }
__attribute__((noinline)) static double in_mul_normal(void) { return den * 1e300; }
__attribute__((noinline)) static float fout_div(void) { return fmin_ / 3.0f; }
__attribute__((noinline)) static double avx_out(void)
{
    double r[4];
    _mm256_storeu_pd(r, _mm256_mul_pd(_mm256_set1_pd(dmin), _mm256_set_pd(0.5, 0.25, 2.0, 0.125)));
    return r[0] + r[1] + r[2] + r[3];
}

static double phase(void)
{
    double s = 0;
    for (int k = 0; k < 100; k++)
        s += out_div() + out_mul() + out_sub() + in_add() + in_mul_normal() + fout_div() + avx_out();
    return s;
}

int main(void)
{
    printf("out_div        %a\n", out_div());
    printf("out_mul        %a\n", out_mul());
    printf("out_sub        %a\n", out_sub());
    printf("in_add         %a\n", in_add());
    printf("in_mul_normal  %a\n", in_mul_normal());
    printf("fout_div       %a\n", (double)fout_div());
    printf("avx_out        %a\n", avx_out());
    fflush(stdout);
    VERROU_PRINT_DENORM_COUNTER;
    VERROU_RESET_DENORM_COUNTER;
    printf("phase          %a\n", phase());
    return 0;
}
