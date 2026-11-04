/* --gen-source only lists the lines with a floating-point operation : the
   integer lines following a float line in the same IRSB are not listed. */
#include <stdio.h>

volatile int counter=1;

__attribute__((noinline, optimize("O0")))
double compute (double a, double b) {
  double res=a*b; /* the only line with an operation of this function */
  counter=counter+1;
  counter=counter*3;
  counter=counter-2;
  return res;
}

int main (int argc, char** argv) {
  double res=compute (1.5, 2.5);
  printf ("%f %d\n", res, counter);
  return 0;
}
