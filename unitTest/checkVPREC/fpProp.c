#include "fpPropImpl.c"

int main(int argc, char** argv){
   if(!((argc==3)|| (argc==4) )){
      printf("usage ./propFP range precision\n");
      return EXIT_FAILURE;
   }
   
   int range=atoi(argv[1]);
   int precision=atoi(argv[2]);
   double value=1.;
   if( argc==4){
      value=atof(argv[3]);
   }
   
   printf("emax: %d\n", emax(range));
   printf("emin: %d\n", emin(range));

   printf("max: %.17e\n",floatMax(range,precision));
   printf("minNorm: %.17e\n", floatMinNorm(range,precision));
   printf("minDeNorm: %.17e\n", floatMinDeNorm(range,precision));

   printf("ulpOne: %.17e %a\n", ulpOne(precision),ulpOne(precision));
   printf("ulp   : %.17e %a\n", getUlp(value,range,precision),getUlp(value,range,precision));

}
