#include "fpPropImpl.c"
#include <assert.h>
#include <math.h>

int main(int argc, char** argv){
   if(!((argc==3)|| (argc==4) )){
      printf("usage ./propFP range precision [value]\n");
      printf("\tfor float  ./propFP 8 23\n");
      printf("\tfor double  ./propFP 11 52\n");
      return EXIT_FAILURE;
   }

   int range=atoi(argv[1]);
   int precision=atoi(argv[2]);
   double value=0.;
   if( argc==4){
      value=atof(argv[3]);
   }

   assert(range>1);
   assert(range<=11);
   assert(precision>1);
   assert(precision<=52);

   printf("emax: %ld\n", emax(range));
   printf("emin: %ld\n", emin(range));

   printf("max: %.17e\n",floatMax(range,precision));
   double minNorm=floatMinNorm(range,precision);
   printf("minNorm: %.17e\n", minNorm);
   double minDenorm=floatMinDeNorm(range,precision);
   printf("minDeNorm: %.17e\n", minDenorm);

   assert(minDenorm < minNorm);

   double ulp1=ulpOne(precision);
   printf("ulpOne: %.17e %a\n", ulp1,ulp1);

   double ulpValue;
   if(value!=0.){
      int64_t expA=getExp(value);
      printf("exp(%e | %a) : %ld\n", value, value,expA);
      ulpValue=getUlp(value,range,precision);
      printf("ulp(%e) : %.17e %a\n", value,ulpValue,ulpValue);
      assert(ulpValue!=0);
   }

   assert(getUlp(1.,range, precision) ==  ulp1);

   double ulpMinDenorm=getUlp(minDenorm,range, precision);
   printf("ulpMinDenorm %.17e %a\n", ulpMinDenorm, ulpMinDenorm);
   assert(minDenorm == ulpMinDenorm);

   if(range==8 && precision==23 && value!=0){//float check
      float af=fabs(value);
      float next=nextafterf(af, INFINITY);
      float ulp=next-af;
      assert(ulp==ulpValue);
      printf("check float ulp OK\n");
   }
   if(range==11 && precision==52&& value!=0){//float check
      double af=fabs(value);
      double next=nextafter(af, INFINITY);
      double ulp=next-af;
      assert(ulp==ulpValue);
      printf("check double ulp\n");
   }
}
