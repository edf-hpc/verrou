#include "../verrou.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main (int argc, char** argv) {
   volatile float af=0.111;
   volatile double a=0.111;

   if(argc==2){
      a=atof(argv[1]);
      af=(float) a;
   }

   // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;
  float cosf_uninst= cosf(af);
  double cos_uninst= cos(a);
  VERROU_START_INSTRUMENTATION;
  float cosf_inst= cosf(af);
  double cos_inst= cos(a);
  VERROU_STOP_INSTRUMENTATION;

  printf("a: %.17f\n",a);
  printf("af: %.17f\n",af);
  printf("cosf uninst: %.17f\n",cosf_uninst);
  printf("cos  uninst: %.17f\n",cos_uninst);
  printf("cosf   inst: %.17f\n",cosf_inst);
  printf("cos    inst: %.17f\n",cos_inst);

  if( cosf_inst!=cosf_uninst){
     printf("cosf impacted by instrumentation\n");
  }

  if( cos_inst!=cos_uninst){
     printf("cos impacted by instrumentation\n");
  }

  if(cosf_uninst==cos_uninst){
     printf("native cos and cosf are equal : strange\n");
  }

  if(cosf_inst==cos_inst){
     printf("instrumented cos and cosf are equal\n");
  }else{
     printf("instrumented cos and cosf are different\n");
  };

  return RUNNING_ON_VALGRIND;
}
