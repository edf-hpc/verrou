#include "../verrou.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int N = 100000;
float epsilon = 1e-9;

float compute () {
  float sum = 1;
  int i;
  VERROU_START_INSTRUMENTATION;
  for (i = 0 ; i < N ; ++i) {
    sum += epsilon;
  }
  VERROU_STOP_INSTRUMENTATION;

  return sum;
}

int main (int argc, char **argv) {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use
  float res = compute();
  float ref = 1 + N*epsilon;

  if (!strcmp (argv[1], "0")) {
    // Just print the result
    printf ("%.10f\n", res);
  }
  else if (!strcmp (argv[1], "1")) {
    // sum1.vgtest
    // CL switches:
    //   --instr-atstart=no
    //
    // res should be 1
    if (res == 1) {
      printf ("OK\n");
    } else {
      printf ("%.10f\n", res);
    }
  }
  else if (!strcmp (argv[1], "2")) {
    // sum2.vgtest
    // CL switches:
    //   --instr-atstart=no
    //   --rounding-mode=random
    //
    // res should be significantly different from 1,
    // since there are floating point errors
    float threshold = 20*N*epsilon;
    if (fabs(res - 1) > threshold) {
      printf ("OK\n");
    } else {
      printf ("error: |%.10e| < %.10e\n", res-1, threshold);
    }
  }
  else if (!strcmp (argv[1], "3")) {
    // sum3.vgtest
    // CL switches:
    //   --instr-atstart=no
    //   --rounding-mode=nearness
    //
    // res should be close to ref
    float threshold = 0.2f * (float)N * epsilon;
    if (fabs(res-ref) < threshold) {
      printf ("OK\n");
    } else {
      printf ("|%.10e| > %.10e \n", res-ref, threshold);
    }
  }

  return RUNNING_ON_VALGRIND;
}
