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
  if (compute() == (float)4950.)
    printf ("OK\n");

  // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;
  if (compute() == (float)4950.)
    printf ("OK\n");
  VERROU_START_INSTRUMENTATION;

  if (compute() == (float)4950.)
    printf ("OK\n");

  return RUNNING_ON_VALGRIND;
}
