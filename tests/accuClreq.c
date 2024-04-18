#include "../verrou.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>


float compute () {
  float res=0;
  int i;
  for(i=0; i< 1000; i++){
    res+=(float)0.1;
  }
  return res-100 ;
}

int main (int argc, char** argv) {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

  bool hard=false;
  bool soft=false;

  if(argc!=2){
     return RUNNING_ON_VALGRIND;
  }else{
     if(strcmp("hard",argv[1])==0){
        hard=true;
     }
     if(strcmp("soft",argv[1])==0){
        soft=true;
     }
     if(strcmp("both",argv[1])==0){
        soft=true; hard=true;
     }
  }
  if(soft==false && hard==false){
     printf ("error invalid data\n");
  }

  float res_init=compute();

  // Uninstrumented part
  if(hard && !soft){ VERROU_STOP_INSTRUMENTATION;}
  if(!hard && soft){ VERROU_STOP_SOFT_INSTRUMENTATION;}
  if(hard && soft){
     VERROU_START_INSTRUMENTATION;
     VERROU_STOP_SOFT_INSTRUMENTATION;
  }

  float res_uninst=compute();

  //Instrumented part
  if(hard){ VERROU_START_INSTRUMENTATION;}
  if(soft){ VERROU_START_SOFT_INSTRUMENTATION;}

  float res_end=compute();

  printf ("%f %f %f\n", res_init, res_uninst, res_end);

  if (res_init == res_uninst){
    printf ("NO INSTR AT START\n");
  }else{
    printf ("INSTR AT START\n");
  }

  if (res_init == res_end){
    printf ("DET\n");
  }else{
    printf( "NO_DET\n");
  }

  if (res_end == res_uninst){
    printf ("START FAIL\n");
  }else{
    printf ("START SUCCEED\n");
  }

  return RUNNING_ON_VALGRIND;
}
