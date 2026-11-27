/* R1 reference corpus: all the floating-point operations instrumented by
 * verrou, on special values and on fixed pseudo-random values.
 * Each result is printed exactly (%a): the output only depends on the
 * rounding mode, it is a bitwise reference.
 *
 * Build: -O1 -mavx2 -mfma -ffp-contract=off (no implicit contraction:
 * every FMA of the corpus is explicit). */
#include <float.h>
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* volatile: prevents compile-time evaluation */
static volatile double vd[3];
static volatile float vf[3];

static uint64_t lcg_state = 0x9E3779B97F4A7C15ULL;
static uint64_t lcg(void)
{
    lcg_state = lcg_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return lcg_state >> 11;
}
static double rnd(void) /* in [-1e3, 1e3], all the significand bits used */
{
    return ((double)lcg() / (double)(1ULL << 53) - 0.5) * 2e3;
}

static void pd(const char *op, int k, double x) { printf("%s %d %a\n", op, k, x); }
static void pf(const char *op, int k, float x) { printf("%s %d %a\n", op, k, (double)x); }

#define NSPEC 24
static const double spec[NSPEC] = {
    0.0, -0.0, 1.0, -1.0, 3.0, 0.1, 1.0 / 3.0, 1e16, 1.0 + 0x1p-52, 0x1p-1022, 0x1p-1074,
    0x1p-1022 / 3.0, DBL_MAX, -DBL_MAX, 1e308, 1e-308, 2.0 / 3.0, 1e-5, 123456.789, -7.25,
    0x1.fffffffffffffp+1023, 0x1p-537, 1.0 - 0x1p-53, 4503599627370496.5,
};

static void scalar_double(int k, double a, double b, double c)
{
    vd[0] = a, vd[1] = b, vd[2] = c;
    pd("dadd", k, vd[0] + vd[1]);
    pd("dsub", k, vd[0] - vd[1]);
    pd("dmul", k, vd[0] * vd[1]);
    pd("ddiv", k, vd[0] / vd[1]);
    pd("dsqrt", k, sqrt(fabs(vd[0])));
    pd("dfma", k, fma(vd[0], vd[1], vd[2]));
    pd("dfms", k, fma(vd[0], vd[1], -vd[2]));
    pd("dcvt2f", k, (double)(float)vd[0]);
    long long i = (long long)(vd[1] * 1e3);
    pd("dcvti", k, (double)(i * 1000003LL)); /* int64 -> double, rounded */
}

static void scalar_float(int k, float a, float b, float c)
{
    vf[0] = a, vf[1] = b, vf[2] = c;
    pf("fadd", k, vf[0] + vf[1]);
    pf("fsub", k, vf[0] - vf[1]);
    pf("fmul", k, vf[0] * vf[1]);
    pf("fdiv", k, vf[0] / vf[1]);
    pf("fsqrt", k, sqrtf(fabsf(vf[0])));
    pf("ffma", k, fmaf(vf[0], vf[1], vf[2]));
    int i = (int)(vf[1] * 1e3f);
    pf("fcvti", k, (float)(i * 1000003)); /* int32 -> float, rounded */
}

static void packed(int k, const double *a, const double *b, const double *c)
{
    double r[4];
    float rf[8];
    /* SSE2: 2 doubles */
    __m128d xa = _mm_loadu_pd(a), xb = _mm_loadu_pd(b);
    _mm_storeu_pd(r, _mm_add_pd(xa, xb));
    pd("sse_addpd0", k, r[0]), pd("sse_addpd1", k, r[1]);
    _mm_storeu_pd(r, _mm_mul_pd(xa, xb));
    pd("sse_mulpd0", k, r[0]), pd("sse_mulpd1", k, r[1]);
    _mm_storeu_pd(r, _mm_div_pd(xa, xb));
    pd("sse_divpd0", k, r[0]), pd("sse_divpd1", k, r[1]);
    _mm_storeu_pd(r, _mm_sqrt_pd(_mm_andnot_pd(_mm_set1_pd(-0.0), xa)));
    pd("sse_sqrtpd0", k, r[0]), pd("sse_sqrtpd1", k, r[1]);
    /* SSE: 4 floats */
    __m128 fa = _mm_cvtpd_ps(xa), fb = _mm_cvtpd_ps(xb);
    fa = _mm_movelh_ps(fa, _mm_cvtpd_ps(_mm_loadu_pd(c)));
    fb = _mm_movelh_ps(fb, _mm_cvtpd_ps(_mm_loadu_pd(a + 2)));
    _mm_storeu_ps(rf, _mm_add_ps(fa, fb));
    for (int l = 0; l < 4; l++)
        pf("sse_addps", 4 * k + l, rf[l]);
    _mm_storeu_ps(rf, _mm_div_ps(fa, fb));
    for (int l = 0; l < 4; l++)
        pf("sse_divps", 4 * k + l, rf[l]);
    /* AVX: 4 doubles, FMA */
    __m256d ya = _mm256_loadu_pd(a), yb = _mm256_loadu_pd(b), yc = _mm256_loadu_pd(c);
    _mm256_storeu_pd(r, _mm256_add_pd(ya, yb));
    for (int l = 0; l < 4; l++)
        pd("avx_addpd", 4 * k + l, r[l]);
    _mm256_storeu_pd(r, _mm256_sub_pd(ya, yb));
    for (int l = 0; l < 4; l++)
        pd("avx_subpd", 4 * k + l, r[l]);
    _mm256_storeu_pd(r, _mm256_mul_pd(ya, yb));
    for (int l = 0; l < 4; l++)
        pd("avx_mulpd", 4 * k + l, r[l]);
    _mm256_storeu_pd(r, _mm256_div_pd(ya, yb));
    for (int l = 0; l < 4; l++)
        pd("avx_divpd", 4 * k + l, r[l]);
    _mm256_storeu_pd(r, _mm256_fmadd_pd(ya, yb, yc));
    for (int l = 0; l < 4; l++)
        pd("avx_fmadd", 4 * k + l, r[l]);
    _mm256_storeu_pd(r, _mm256_fnmadd_pd(ya, yb, yc));
    for (int l = 0; l < 4; l++)
        pd("avx_fnmadd", 4 * k + l, r[l]);
    /* AVX: 8 floats */
    __m256 za = _mm256_set_m128(fb, fa), zb = _mm256_set_m128(fa, fb);
    _mm256_storeu_ps(rf, _mm256_mul_ps(za, zb));
    for (int l = 0; l < 8; l++)
        pf("avx_mulps", 8 * k + l, rf[l]);
    _mm256_storeu_ps(rf, _mm256_fmadd_ps(za, zb, za));
    for (int l = 0; l < 8; l++)
        pf("avx_fmaddps", 8 * k + l, rf[l]);
    _mm256_storeu_ps(rf, _mm256_sqrt_ps(_mm256_andnot_ps(_mm256_set1_ps(-0.0f), za)));
    for (int l = 0; l < 8; l++)
        pf("avx_sqrtps", 8 * k + l, rf[l]);
}

int main(void)
{
    /* line-buffered output for IOMatch (start/stop case of the corpus) */
    if (getenv("CORPUS_LINEBUF") != NULL)
        setvbuf(stdout, NULL, _IOLBF, 0);
    int k = 0;
    /* special values: all the (a, b) pairs, c rotating */
    for (int i = 0; i < NSPEC; i++)
        for (int j = 0; j < NSPEC; j++, k++) {
            double c = spec[(i + 2 * j + 1) % NSPEC];
            scalar_double(k, spec[i], spec[j], c);
            scalar_float(k, (float)spec[i], (float)spec[j], (float)c);
        }
    /* pseudo-random values, including cancellations (b close to -a) */
    for (int i = 0; i < 200; i++, k++) {
        double a = rnd(), b = (i % 4 == 0) ? -a * (1.0 + 0x1p-40 * rnd()) : rnd(), c = rnd();
        scalar_double(k, a, b, c);
        scalar_float(k, (float)a, (float)b, (float)c);
    }
    /* packed */
    double a[4], b[4], c[4];
    for (int i = 0; i < 100; i++) {
        for (int l = 0; l < 4; l++) {
            a[l] = (i < NSPEC) ? spec[(i + l) % NSPEC] : rnd();
            b[l] = (i % 3 == 0) ? -a[l] * (1.0 + 0x1p-35 * l) : rnd();
            c[l] = rnd();
        }
        packed(i, a, b, c);
    }
    return 0;
}
