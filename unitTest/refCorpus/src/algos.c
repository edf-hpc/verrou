/* R1 reference corpus: common numerical algorithms, sensitive to rounding
 * to various degrees. Usage: algos NTHREADS
 * Each thread computes the same kernels on its own data (seed = thread
 * number); the results are printed exactly (%a), in thread order once
 * all threads have finished: the output is deterministic for the
 * deterministic rounding modes, whatever the scheduling.
 *
 * Build: -O2 -mavx2 -mfma (compiler vectorization and FMA contraction,
 * as in a real code). */
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NRES 32
#define N 4096

typedef struct {
    int id;
    double res[NRES];
    int nres;
} job_t;

static uint64_t lcg(uint64_t *s)
{
    *s = *s * 6364136223846793005ULL + 1442695040888963407ULL;
    return *s >> 11;
}
static double rnd(uint64_t *s) { return (double)lcg(s) / (double)(1ULL << 53); }

static void put(job_t *j, double x) { j->res[j->nres++] = x; }

static void *kernels(void *p)
{
    job_t *j = p;
    uint64_t s = 0x1234567ULL + 977ULL * (uint64_t)j->id;
    static __thread double x[N], y[N], z[N];
    for (int i = 0; i < N; i++) {
        x[i] = rnd(&s) - 0.5;
        y[i] = rnd(&s) - 0.5;
    }
    /* sums: naive double, naive float, compensated (Kahan) */
    double sd = 0, kc = 0, ks = 0;
    float sf = 0;
    for (int i = 1; i <= 100000; i++) {
        double t = 1.0 / ((double)i * i);
        sd += t;
        sf += (float)t;
        double yk = t - kc, tk = ks + yk;
        kc = (tk - ks) - yk;
        ks = tk;
    }
    put(j, sd), put(j, sf), put(j, ks);
    /* dot product (vectorized by the compiler) and norm */
    double dot = 0, nrm = 0;
    for (int i = 0; i < N; i++) {
        dot += x[i] * y[i];
        nrm += x[i] * x[i];
    }
    put(j, dot), put(j, sqrt(nrm));
    /* repeated axpy */
    for (int r = 0; r < 50; r++)
        for (int i = 0; i < N; i++)
            z[i] = 0.999 * z[i] + 1e-3 * x[i] * y[i];
    double sz = 0;
    for (int i = 0; i < N; i++)
        sz += z[i];
    put(j, sz);
    /* Horner, degree 12 polynomial (cancellations near the root 1) */
    double hs = 0;
    for (int k = 0; k < 200; k++) {
        double t = 0.99 + 0.0001 * k, h = 0;
        static const double coef[13] = { 1, -12, 66, -220, 495, -792, 924, -792, 495, -220, 66, -12, 1 };
        for (int d = 0; d < 13; d++)
            h = h * t + coef[d];
        hs += fabs(h);
    }
    put(j, hs);
    /* Muller recurrence (unstable) */
    double u0 = 2.0, u1 = -4.0;
    for (int k = 0; k < 25; k++) {
        double u2 = 111.0 - 1130.0 / u1 + 3000.0 / (u0 * u1);
        u0 = u1, u1 = u2;
    }
    put(j, u1);
    /* Newton for sqrt(2 + id) */
    double r = 1.0;
    for (int k = 0; k < 8; k++)
        r = 0.5 * (r + (2.0 + j->id) / r);
    put(j, r);
    /* Gauss without pivoting on an 8x8 Hilbert matrix (ill-conditioned) */
    double A[8][9];
    for (int a = 0; a < 8; a++) {
        for (int b = 0; b < 8; b++)
            A[a][b] = 1.0 / (a + b + 1 + j->id);
        A[a][8] = 1.0;
    }
    for (int p = 0; p < 8; p++)
        for (int a = p + 1; a < 8; a++) {
            double f = A[a][p] / A[p][p];
            for (int b = p; b < 9; b++)
                A[a][b] -= f * A[p][b];
        }
    double sol[8];
    for (int a = 7; a >= 0; a--) {
        double t = A[a][8];
        for (int b = a + 1; b < 8; b++)
            t -= A[a][b] * sol[b];
        sol[a] = t / A[a][a];
    }
    put(j, sol[0]), put(j, sol[7]);
    /* 2D heat stencil, 32x32, 100 steps */
    static __thread double g[34][34], h2[34][34];
    memset(g, 0, sizeof g);
    g[16][16] = 1000.0 + j->id;
    for (int st = 0; st < 100; st++) {
        for (int a = 1; a < 33; a++)
            for (int b = 1; b < 33; b++)
                h2[a][b] = g[a][b] + 0.2 * (g[a - 1][b] + g[a + 1][b] + g[a][b - 1] + g[a][b + 1] - 4.0 * g[a][b]);
        memcpy(g, h2, sizeof g);
    }
    put(j, g[16][16]), put(j, g[10][20]);
    /* libm (excluded by default by verrou): the result depends on what is
     * perturbed around it */
    double lm = 0;
    for (int k = 1; k < 200; k++)
        lm += exp(-0.01 * k) * sin(0.1 * k) + log(1.0 + k * fabs(x[k]));
    put(j, lm);
    return NULL;
}

int main(int argc, char **argv)
{
    if (getenv("CORPUS_LINEBUF") != NULL)
        setvbuf(stdout, NULL, _IOLBF, 0);
    int nt = argc > 1 ? atoi(argv[1]) : 1;
    if (nt < 1 || nt > 16)
        nt = 1;
    job_t jobs[16];
    pthread_t th[16];
    for (int t = 0; t < nt; t++) {
        memset(&jobs[t], 0, sizeof jobs[t]);
        jobs[t].id = t;
        if (nt == 1)
            kernels(&jobs[t]);
        else
            pthread_create(&th[t], NULL, kernels, &jobs[t]);
    }
    for (int t = 0; t < nt; t++) {
        if (nt > 1)
            pthread_join(th[t], NULL);
        for (int k = 0; k < jobs[t].nres; k++)
            printf("thread %d res %d %a\n", t, k, jobs[t].res[k]);
    }
    return 0;
}
