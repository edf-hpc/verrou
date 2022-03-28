#include <stdio.h>
#include <string.h>
#include <math.h>

#define    USE_VERROU_FMA
#include "../interflop_backends/interflop_verrou/vr_fma.hxx"

int main (int argc, char **argv) {

   double a=1.e-160;
   double b=1.e-160;

   float af=1.e-20;
   float bf=-1.e-20;

   double a1=1.e-320;
   double a2=-1.;
   double a3=-1.1e-320;

   printf("c: %e\n", a*b);
   printf("cf: %e\n", af*bf);
   printf("cfma: %e\n", vr_fma(a1,a2,a3));
      
   
   
   
   return 0;
}
