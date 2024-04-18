#include "../verrou.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

float compute () {
  int i;
  float sum = 0;
  for (i = 0 ; i<100 ; ++i) {
    sum += (float)i;
  }
  return sum;
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

  if (compute() == (float)4950.)
    printf ("OK\n");

  // Uninstrumented part
  if(hard && !soft){ VERROU_STOP_INSTRUMENTATION;}
  if(!hard && soft){ VERROU_STOP_SOFT_INSTRUMENTATION;}
  if(hard && soft){
     VERROU_START_INSTRUMENTATION;
     VERROU_STOP_SOFT_INSTRUMENTATION;
  }

  if (compute() == (float)4950.)
    printf ("OK\n");

  //Instrumented part
  if(hard){ VERROU_START_INSTRUMENTATION;}
  if(soft){ VERROU_START_SOFT_INSTRUMENTATION;}

  if (compute() == (float)4950.)
    printf ("OK\n");

  return RUNNING_ON_VALGRIND;
}
