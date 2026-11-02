// --rounding-mode=random_instr --libm=instrumented : the direction of a libm function only depends
// on the seed and on the function and its type. With an argument, prints the results.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../verrou.h"

#define NB_SEEDS 64
#define NB_ITER 100

static volatile double one=1.;
static volatile float oneFloat=1.f;

// The result of a function does not change during an execution
static double stableCos (void) {
   const double res=cos (one);
   for(int i=0 ; i<NB_ITER ; i++){
      if(cos (one)!=res){
         printf("direction changed during the execution\n");
         exit(1);
      }
   }
   return res;
}

int main (int argc, char** argv) {
   if(argc>1){
      printf("%a %a %a\n", stableCos (), exp (one), (double)cosf (oneFloat));
      return 0;
   }
   double results[NB_SEEDS];
   int nbDiffer=0;
   int nbDifferExp=0;
   int nbDifferFloat=0;
   VERROU_SET_SEED (0);
   const double firstCos=stableCos ();
   const double firstExp=exp (one);
   const double firstFloat=cosf (oneFloat);
   for(int seed=0 ; seed<NB_SEEDS ; seed++){
      VERROU_SET_SEED (seed);
      results[seed]=stableCos ();
      nbDiffer+= results[seed]!=firstCos;
      nbDifferExp+= (exp (one)!=firstExp) != (results[seed]!=firstCos);
      nbDifferFloat+= (cosf (oneFloat)!=firstFloat) != (results[seed]!=firstCos);
   }
   int reproducible=1;
   for(int seed=NB_SEEDS-1 ; seed>=0 ; seed--){
      VERROU_SET_SEED (seed);
      reproducible= reproducible && results[seed]==stableCos ();
   }
   printf("the seed selects the direction: %d\n", nbDiffer>0 && nbDiffer<NB_SEEDS);
   printf("the function selects the direction: %d\n", nbDifferExp>0 && nbDifferExp<NB_SEEDS);
   printf("the type selects the direction: %d\n", nbDifferFloat>0 && nbDifferFloat<NB_SEEDS);
   printf("reproducible: %d\n", reproducible);
   return 0;
}
