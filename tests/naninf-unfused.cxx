#include <math.h>
#include <stdio.h>
#include "fma_common.hxx"

// With --unfused=yes the multiplication gives +Inf and the addition gives NaN :
// only the final result is checked, so only the NaN is reported.
int main(){
  double a=1e200, b=1e200, c=-INFINITY;
  double res=intrin_fma<double>(a,b,c);
  printf("%d\n", isnan(res) ? 1 : 0);
  return 0;
}
