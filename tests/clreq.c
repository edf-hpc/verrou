#include "../verrou.h"
#include <stdio.h>

float compute () {
  int i;
  float sum = 0;
  for (i = 0 ; i<100 ; ++i) {
    sum += (float)i;
  }
  return sum;
}

int main () {
  printf ("%f\n", compute ());

  // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;
  printf ("%f\n", compute ());
  VERROU_START_INSTRUMENTATION;

  printf ("%f\n", compute ());

  return RUNNING_ON_VALGRIND;
}
