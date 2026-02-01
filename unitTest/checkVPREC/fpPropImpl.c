#include <stdio.h>
#include "../../interflop/common/float_const.h"
#include "../../interflop/common/float_struct.h"
#include <assert.h>

int emax(int range){
   int res= (1 << (range - 1)) - 1;
   return res;
   }

int emin(int range){
  int res = 1 - emax(range);
  return res;
}

int getExp(double a){
  binary64 aexp = {.f64 = a};
  aexp.s64 = (int64_t)((DOUBLE_GET_EXP & aexp.u64) >> DOUBLE_PMAN_SIZE) -
             DOUBLE_EXP_COMP;
  return aexp.s64;
}

double getUlp(double a, int range, int precision){
   int myemax=emax(range);
   int myemin=emin(range);
   int64_t expo=getExp(a);

   assert(expo<=myemax);

   int64_t expoUlp=expo-precision;
   if(expoUlp>=myemin){
      if(expoUlp>=emin(12)){
         binary64 res;
         res.ieee.sign = 0;
         res.ieee.exponent = expoUlp+1023;
         res.ieee.mantissa = 0;
         return res.f64;
      }else{
         printf("Ulp Denormal\n");
      }
      return 0.;
   }else{
      expoUlp=myemin-precision;
      if(expoUlp>=emin(12)){
         binary64 res;
         res.ieee.sign = 0;
         res.ieee.exponent = expoUlp+1023;
         res.ieee.mantissa = 0;
         return res.f64;
      }else{
         printf("Ulp Denormal\n");
      }
      return 0;
   }
}

double floatMax(int range, int precision){
  int64_t one=1;
  int64_t mantissa=0;
  for(int shift=0; shift< precision; shift++){
     mantissa+= (one << (DOUBLE_PMAN_SIZE -1 -shift)) ;
  }

  binary64 res;
  res.ieee.sign = 0;
  res.ieee.exponent = emax(range)+1023;
  res.ieee.mantissa = mantissa;
  return res.f64;
}

double floatMinNorm(int range, int precision){
  binary64 res;
  res.ieee.sign = 0;
  res.ieee.exponent = emin(range)+1023;
  res.ieee.mantissa = 0;
  return res.f64;
}

double floatMinDeNorm(int range, int precision){
   binary64 res;
   res.ieee.sign = 0;
   res.ieee.exponent = emin(range)+1023 - precision;
   res.ieee.mantissa = 0;
   return res.f64;
}


double ulpOne(int precision){
   binary64 res;
   res.ieee.sign = 0;
   res.ieee.exponent = 0+1023 - precision;
   res.ieee.mantissa = 0;
   return res.f64;
}
