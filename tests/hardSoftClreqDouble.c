#include "../verrou.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

float computeFloat () {
  float res=0;
  int i;
  for(i=0; i< 1000; i++){
     res+=(float)0.1;
  }
  return res-100 ;
}


double compute () {
  double res=0;
  int i;
  for(i=0; i< 1000; i++){
    res+=0.1;
  }
  return res-100 ;
}

void detect(int index, double* res, double res_init, float res_float){
   printf("res[%d] =%.17f\n",index, res[index]);
   if((res[index] - res_init) ==0){
      printf("==\n"); //fflush(stdout);
   }else{
      printf("!=\n"); //fflush(stdout);
   }
   if((res[index] - res_float) ==0){
      printf("float\n"); //fflush(stdout);
   }else{
      printf("double\n"); //fflush(stdout);
   }
   printf("\n");
}

int main (int argc, char** argv) {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

  double res[10];
  double res_init=compute();
  float res_float=computeFloat();
  if(res_init == res_float){
     printf("res_init == res_float\n");
  }
  
  VERROU_STOP_SOFT_INSTRUMENTATION;
  VERROU_START_INSTRUMENTATION;
  res[0]=compute();  //non inst
  detect(0, res, res_init, res_float);
  VERROU_STOP_INSTRUMENTATION;
  VERROU_START_SOFT_INSTRUMENTATION;
  res[1]=compute(); //non inst
  detect(1, res, res_init,res_float);
  VERROU_START_INSTRUMENTATION;
  res[2]=compute(); // inst
  detect(2, res, res_init, res_float);
  VERROU_STOP_SOFT_INSTRUMENTATION;
  res[3]=compute(); // non inst
  detect(3, res, res_init, res_float);
  VERROU_START_SOFT_INSTRUMENTATION;
  res[4]=compute(); // inst
  detect(4, res, res_init, res_float);
  VERROU_STOP_SOFT_INSTRUMENTATION;
  res[5]=compute(); //no inst
  detect(5, res, res_init, res_float);
  VERROU_STOP_INSTRUMENTATION;
  res[6]=compute(); //no inst
  detect(6, res, res_init, res_float);
  VERROU_START_INSTRUMENTATION;
  res[7]=compute(); //no inst
  detect(7, res, res_init, res_float);
  VERROU_START_SOFT_INSTRUMENTATION;
  res[8]=compute(); //inst
  detect(8, res, res_init, res_float);
  VERROU_START_SOFT_INSTRUMENTATION;
  res[9]=compute(); //inst
  detect(9, res, res_init, res_float);
  return RUNNING_ON_VALGRIND;
}
