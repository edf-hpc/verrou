#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>


#include "../interflop_backends/interflop_verrou/vr_fma.hxx"

int main (int argc, char **argv) {
   double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

   double a=1.e-160;
   double b=1.e-160;

   float af=1.e-20;
   float bf=-1.e-20;

   double a1=1.e-320;
   double a2=-1.;
   double a3=-1.1e-320;

   std::cout << "init" << std::endl;
   double c=a*b;
   std::cout << "after axb" << std::endl;
   double cf=af*bf;
   std::cout << "after afxbf" << std::endl;
#ifdef    USE_VERROU_FMA
   double cfma=vr_fma(a1,a2,a3);
#else
   double cfma=fma(a1,a2,a3);
#endif
   printf("c: %e\n", c);
   printf("cf: %e\n", cf);
   printf("cfma: %e\n",cfma );
      
   {
     double am1=1.e-320;
     double am2=1.e100;
     double resm=am1*am2;
     double resp=am1+am2;
     printf("mul: %e\n",resm );
     printf("add: %e\n",resp );
   }
   
   
   return 0;
}
