/* R1 reference corpus: verrou client requests (verrou.h).
 * A small kernel is computed between client requests; its results (%a) and
 * the values returned by the requests are printed. The messages verrou
 * writes in its log are part of the reference too.
 *
 * verrou.h uses the types vr_RoundingMode and Bool (a Valgrind internal
 * type) without defining them: they are defined here.
 * VERROU_SET_ROUNDING_MODE takes the display name of the mode (a string in
 * capitals, "UPWARD", not the "upward" of --rounding-mode=; an integer
 * crashes valgrind), while VERROU_GET_ROUNDING returns the value of enum
 * vr_RoundingMode (interflop_verrou.h), not exported by verrou.h. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef int vr_RoundingMode;
typedef unsigned char Bool;
#include "verrou.h"

static volatile double vin = 0.1;

static double kernel(void)
{
    double s = 0, x = vin;
    for (int i = 1; i <= 1000; i++)
        s += x / i + 1e-3 * x * i;
    return s;
}

/* Deterministic section: the seed is derived from the call site of the
 * request and from the pid (vr_deterministic_section_hash), so the result
 * differs between runs but is the same each time the section is entered in
 * a run: the reference records this property, not the value. */
static double det_kernel(void)
{
    VERROU_START_DETERMINISTIC(0);
    double r = kernel();
    VERROU_STOP_DETERMINISTIC(0);
    return r;
}

static void step(const char *label)
{
    printf("%-28s kernel=%a instrumented=%u not_instrumented=%u rounding=%d\n", label, kernel(),
           VERROU_COUNT_FP_INSTRUMENTED, VERROU_COUNT_FP_NOT_INSTRUMENTED, (int)VERROU_GET_ROUNDING);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("is_instrumented float=%d double=%d count_op=%d float_conv=%d\n", (int)VERROU_IS_INSTRUMENTED_FLOAT,
           (int)VERROU_IS_INSTRUMENTED_DOUBLE, (int)VERROU_COUNT_OP, (int)VERROU_FLOAT_CONV);
    step("start");
    /* hard and soft start/stop */
    VERROU_STOP_INSTRUMENTATION;
    step("stop");
    VERROU_START_INSTRUMENTATION;
    step("start_again");
    VERROU_STOP_SOFT_INSTRUMENTATION;
    step("stop_soft");
    VERROU_START_SOFT_INSTRUMENTATION;
    step("start_soft");
    /* rounding mode changed by the program */
    VERROU_SET_ROUNDING_MODE("UPWARD");
    step("set_rounding upward");
    VERROU_SET_ROUNDING_MODE("TOWARD_ZERO");
    step("set_rounding toward_zero");
    VERROU_SET_ROUNDING_MODE("RANDOM");
    VERROU_SET_SEED(7);
    printf("get_seed after set_seed(7): %llu\n", (unsigned long long)VERROU_GET_SEED);
    step("random seed 7");
    VERROU_SET_SEED(7);
    step("random seed 7 again");
    VERROU_SET_SEED(8);
    step("random seed 8");
    /* deterministic sections */
    double d1 = det_kernel(), d2 = det_kernel(), d3 = kernel();
    printf("deterministic section: same result twice=%d, differs from outside=%d\n", d1 == d2, d1 != d3);
    VERROU_SET_ROUNDING_MODE("RANDOM_DET");
    step("random_det");
    /* prandom */
    VERROU_PRANDOM_UPDATE_VALUE("0.25");
    step("prandom_update_value 0.25");
    VERROU_SET_ROUNDING_MODE("NEAREST");
    step("nearest");
    /* messages in the log */
    VERROU_DISPLAY_COUNTERS;
    VERROU_PRINT_PROFILING_EXACT;
    VERROU_PRINT_DENORM_COUNTER;
    VERROU_RESET_DENORM_COUNTER;
    return 0;
}
