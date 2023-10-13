#include "../verrou.h"
#include <stdio.h>
#include <math.h>
float compute () {
  float res=0;
  int i;
  for(i=0; i< 1000; i++){
    res+=(float)0.1;
  }
  return res-100 ;
}

int main () {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

  float res_init=compute();
  // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;
  float res_uninst=compute();
  VERROU_START_INSTRUMENTATION;

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
