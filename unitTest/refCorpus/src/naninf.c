/* R1 reference corpus: NaN and Inf detection (--check-nan, --check-inf).
 * Each case is a separate function, so that the valgrind errors (with their
 * backtrace) point to a distinct, stable location. The results are printed
 * exactly (%a). Some cases depend on the rounding mode: DBL_MAX + a small
 * value overflows to Inf in upward rounding only.
 * Build: -O1 -g -mavx2 -mfma -ffp-contract=off */
#include <float.h>
#include <immintrin.h>
#include <math.h>
#include <stdio.h>

static volatile double zero = 0.0, one = 1.0, minus_one = -1.0, big = DBL_MAX, tiny = 1e-300;
static volatile float fzero = 0.0f, fbig = FLT_MAX;

__attribute__((noinline)) static double nan_div(void) { return zero / zero; }
__attribute__((noinline)) static double nan_sub(void) { double i = one / zero; return i - i; }
__attribute__((noinline)) static double nan_mul(void) { double i = one / zero; return zero * i; }
__attribute__((noinline)) static double nan_sqrt(void) { return sqrt(minus_one); }
__attribute__((noinline)) static double nan_propagate(void) { double n = zero / zero; return n + one; }
__attribute__((noinline)) static double inf_div(void) { return one / zero; }
__attribute__((noinline)) static double inf_overflow_mul(void) { return big * 2.0; }
__attribute__((noinline)) static double inf_rounding_add(void) { return big + tiny; }
__attribute__((noinline)) static float fnan_div(void) { return fzero / fzero; }
__attribute__((noinline)) static float finf_mul(void) { return fbig * 4.0f; }
__attribute__((noinline)) static double sse_nan(void)
{
    double r[2];
    _mm_storeu_pd(r, _mm_div_pd(_mm_set1_pd(zero), _mm_set_pd(zero, one)));
    return r[0] + r[1] * 0.0;
}
__attribute__((noinline)) static double avx_inf(void)
{
    double r[4];
    _mm256_storeu_pd(r, _mm256_mul_pd(_mm256_set1_pd(big), _mm256_set_pd(1.0, 0.5, 2.0, 4.0)));
    return r[0];
}
__attribute__((noinline)) static double avx_fma_nan(void)
{
    double r[4];
    __m256d i = _mm256_set1_pd(one / zero);
    _mm256_storeu_pd(r, _mm256_fmadd_pd(_mm256_set1_pd(zero), i, _mm256_set1_pd(one)));
    return r[2];
}

int main(void)
{
    struct {
        const char *name;
        double (*f)(void);
    } d[] = {
        { "nan_div", nan_div },   { "nan_sub", nan_sub },         { "nan_mul", nan_mul },
        { "nan_sqrt", nan_sqrt }, { "nan_propagate", nan_propagate }, { "inf_div", inf_div },
        { "inf_overflow_mul", inf_overflow_mul }, { "inf_rounding_add", inf_rounding_add },
        { "sse_nan", sse_nan },   { "avx_inf", avx_inf },         { "avx_fma_nan", avx_fma_nan },
    };
    for (unsigned k = 0; k < sizeof d / sizeof d[0]; k++)
        printf("%-18s %a\n", d[k].name, d[k].f());
    printf("%-18s %a\n", "fnan_div", (double)fnan_div());
    printf("%-18s %a\n", "finf_mul", (double)finf_mul());
    return 0;
}
