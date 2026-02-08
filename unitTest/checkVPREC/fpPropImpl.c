#include <stdio.h>
#include "../../interflop/common/float_const.h"
#include "../../interflop/common/float_struct.h"
#include <assert.h>
#include <math.h>

double floatMinDeNorm(int range, int64_t precision);

int64_t emax(int range){
   int res= (1 << (range - 1)) - 1;
   return res;
   }

int64_t emin(int range){
  int res = 1 - emax(range);
  return res;
}

int64_t getExp(double a){
   int64_t res;
   assert(a!=0.);
   binary64 aexp = {.f64 = a};
   aexp.s64 = (int64_t)((DOUBLE_GET_EXP & aexp.u64) >> DOUBLE_PMAN_SIZE) -
      DOUBLE_EXP_COMP;
   if( (aexp.s64 + DOUBLE_EXP_COMP)!=0){
      res= aexp.s64;
   }else{
      uint64_t one=1;
      uint64_t binaryA=*((uint64_t*)&a);
      int lastIOne=-1;
      for(int i=0; i< DOUBLE_PMAN_SIZE; i++){
         if( (binaryA & (one <<i)) !=0){
            lastIOne=i;
         }
      }
      assert(lastIOne>=0);
      res= -1023 + lastIOne-DOUBLE_PMAN_SIZE +1;
   }

   if( (fabs(a)!=INFINITY) && !(a!=a)){
      int64_t expoMath;
      if(a>0){
         expoMath=floor(log2(a));
      }else{
         expoMath=floor(log2(-a));
      }
      if(expoMath !=res){
         printf("getExp(%a | %.17e)\n",a,a);
         printf("\texpoMath %ld\n", expoMath);
         printf("\tres %ld\n", res);
      }

      assert(expoMath==res);
   }
   return res;
}



double getUlp(double a, int range, int precision){
   int myemax=emax(range);
   int myemin=emin(range);
   if(a==0.){
      return floatMinDeNorm(range, precision);
   }

   int64_t expo=getExp(a);

   if( expo> myemax){
      return INFINITY;
   }

   int64_t expoUlp=expo-precision;
   if(expoUlp<myemin-precision){
      expoUlp=myemin-precision;
   }
   binary64 res;
   if(expoUlp>=emin(11)){
      res.ieee.sign = 0;
      res.ieee.exponent = expoUlp+1023;
      res.ieee.mantissa = 0;
   }else{
      int64_t diffExpoUlp = expoUlp - emin(11);
      int64_t one=1;
      int64_t shift=(DOUBLE_PMAN_SIZE +diffExpoUlp);
      int64_t mantissa;
      if(shift>=0){
         mantissa= (one << (shift));
      }else{
         mantissa=1;
      }
      res.ieee.sign = 0;
      res.ieee.exponent = 0;
      res.ieee.mantissa = mantissa;
   }

   double resM=pow(2, expoUlp);
   if(resM!=res.f64){
      printf("resM %a %.17e\n", resM, resM);
      printf("res.f64 %a %.17e\n", res.f64, res.f64);
   }
   assert(resM==res.f64);

   return res.f64;
}

double floatMax(int range, int precision){
  int64_t one=1;
  int64_t mantissa=0;
  for(int shift=0; shift< precision; shift++){
     mantissa+= (one << (DOUBLE_PMAN_SIZE -1 -shift)) ;
  }

  binary64 res;
  res.ieee.sign = 0;
  res.ieee.exponent = emax(range)+(int64_t)1023;
  res.ieee.mantissa = mantissa;
  return res.f64;
}

double floatMinNorm(int range, int precision){
  binary64 res;
  res.ieee.sign = 0;
  res.ieee.exponent = emin(range)+(int64_t)1023;
  res.ieee.mantissa = 0;
  return res.f64;
}

double floatMinDeNorm(int range, int64_t precision){
   int64_t myexpo=emin(range)+(int64_t)1023 - precision;
   binary64 res;
   if(myexpo>0){
      res.ieee.sign = 0;
      res.ieee.exponent = myexpo;
      res.ieee.mantissa = 0;
      return res.f64;
   }else{
      int64_t one=1;
      int64_t mantissa= one << (DOUBLE_PMAN_SIZE -1 +myexpo );
      res.ieee.sign = 0;
      res.ieee.exponent = 0;
      res.ieee.mantissa = mantissa;
      return res.f64;
   }
}


double ulpOne(int64_t precision){
   binary64 res;
   res.ieee.sign = 0;
   res.ieee.exponent = 0+(int64_t)1023 - precision;
   res.ieee.mantissa = 0;
   return res.f64;
}
