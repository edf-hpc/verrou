// Sign of the rounding error of the double multiplication and division near the underflow and with
// small divisors. Without argument, prints the results. With an argument (for the random rounding
// modes), checks that both directions are reached.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NB_ITER 200

static const uint64_t cases[][2]={
   {0x3fb999999999999aULL, 0x8010000000000000ULL}, // tiny result
   {0x3fb999999999999aULL, 0x0000000000000001ULL},
   {0x3fb999999999999aULL, 0x00000000000007e8ULL},
   {0x3fb999999999999aULL, 0x7fefffffffffffffULL}, // small argument and large result
   {0x3fb999999999999aULL, 0xffefffffffffffffULL},
   {0x0010000000000000ULL, 0x0000000000000001ULL}, // product of two tiny numbers
   {0x8010000000000000ULL, 0x8000000000000001ULL},
   {0x3ff0000000000000ULL, 0x001089ed476ecab0ULL}, // small divisor
   {0xbff0000000000000ULL, 0x001089ed476ecab0ULL},
   {0x000199999999999aULL, 0x4008000000000000ULL}  // tiny dividend
};
#define NB_CASES (sizeof(cases)/sizeof(cases[0]))

static double toDouble (uint64_t bits) {
   double res;
   memcpy(&res, &bits, sizeof(double));
   return res;
}

static uint64_t toBits (double value) {
   uint64_t res;
   memcpy(&res, &value, sizeof(double));
   return res;
}

int main (int argc, char** argv) {
   for(unsigned i=0 ; i<NB_CASES ; i++){
      volatile double a=toDouble (cases[i][0]);
      volatile double b=toDouble (cases[i][1]);
      if(argc==1){
         double mul=a*b;
         double div=a/b;
         printf("%016llx %016llx\n", (unsigned long long)toBits (mul), (unsigned long long)toBits (div));
      }else{
         const double firstMul=a*b;
         const double firstDiv=a/b;
         int mulChanged=0;
         int divChanged=0;
         for(int iter=0 ; iter<NB_ITER ; iter++){
            double mul=a*b;
            double div=a/b;
            mulChanged= mulChanged || toBits (mul)!=toBits (firstMul);
            divChanged= divChanged || toBits (div)!=toBits (firstDiv);
         }
         printf("case %u: %d %d\n", i, mulChanged, divChanged);
      }
   }
   return 0;
}
