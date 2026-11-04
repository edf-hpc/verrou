// A FMA has to give its native result when the instrumentation is stopped, also with --unfused=yes
#include <stdio.h>
#include "fma_common.hxx"

double a=0.1, b=0.3, c=0.7;
float a_f=0.1f, b_f=0.3f, c_f=0.7f;

int main (int argc, char** argv) {
  printf ("double: %a\n", intrin_fma<double>(a,b,c));
  printf ("float: %a\n", (double)intrin_fma<float>(a_f,b_f,c_f));
  return 0;
}
