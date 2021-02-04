#include <stdio.h>
#include <string.h>
#include <math.h>


int main (int argc, char **argv) {

   double a=1.e-160;
   double b=1.e-160;
   double c= a*b ;

   float af=1.e-20;
   float bf=-1.e-20;
   float cf= af*bf;

   double a1=1.e-320;
   double a2=-1.;
   double a3=-1.1e-320;
   double cfma=fma(a1,a2,a3);

   printf("c: %e\n", c);
   printf("cf: %e\n", cf);
   printf("cfma: %e\n", cfma);
      
   
   
   
   return 0;
}
