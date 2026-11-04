// --rounding-mode=random_instr : the direction of an instruction only depends on the seed and on
// the instruction. With an argument, prints the directions of the sites for the exclusion test.
#include <stdio.h>
#include <stdlib.h>
#include "../verrou.h"

#define NB_SEEDS 64
#define NB_ITER 100

static const double thirdDown=0x1.5555555555555p-2, thirdUp=0x1.5555555555556p-2;
static const double tenthDown=0x1.9999999999999p-4, tenthUp=0x1.999999999999ap-4;

__attribute__((noinline)) static double siteA (volatile double* num, volatile double* den) {
   return *num / *den;
}

__attribute__((noinline)) static double siteB (volatile double* num, volatile double* den) {
   return *num / *den;
}

static volatile double one=1., three=3., ten=10.;

static int direction (double res, double down, double up) {
   if(res==down) return 0;
   if(res==up) return 1;
   printf("unexpected result %a\n", res);
   exit(1);
}

static int directionA (void) { return direction (siteA (&one, &three), thirdDown, thirdUp); }
static int directionB (void) { return direction (siteB (&one, &ten), tenthDown, tenthUp); }

// The same instruction is reached from two distinct superblocks
static int stableDirectionA (void) {
   const int res=directionA ();
   for(int i=0 ; i<NB_ITER ; i++){
      if(i%2==0){
         directionB ();
      }
      if(directionA ()!=res){
         printf("direction changed during the execution\n");
         exit(1);
      }
   }
   return res;
}

int main (int argc, char** argv) {
   if(argc>1){
      printf("%d %d\n", stableDirectionA (), directionB ());
      return 0;
   }
   int directions[NB_SEEDS];
   int nbUpA=0;
   int nbDiffer=0;
   for(int seed=0 ; seed<NB_SEEDS ; seed++){
      VERROU_SET_SEED (seed);
      directions[seed]=stableDirectionA ();
      nbUpA+=directions[seed];
      nbDiffer+= directions[seed]!=directionB ();
   }
   int reproducible=1;
   for(int seed=NB_SEEDS-1 ; seed>=0 ; seed--){
      VERROU_SET_SEED (seed);
      reproducible= reproducible && directions[seed]==stableDirectionA ();
   }
   printf("the seed selects the direction: %d\n", nbUpA>0 && nbUpA<NB_SEEDS);
   printf("the instruction selects the direction: %d\n", nbDiffer>0 && nbDiffer<NB_SEEDS);
   printf("reproducible: %d\n", reproducible);
   return 0;
}
