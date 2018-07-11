#include "../verrou.h"
#include <stdio.h>

float compute () {
  float res=0;
  for(int i=0; i< 1000; i++){
    res+=(float)0.1;
  }
  return res-100 ;
}

int main () {
  

  float res_init=compute();
  // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;
  float res_uninst=compute();
  VERROU_START_INSTRUMENTATION;

  float res_end=compute();
  
  printf ("%f %f %f\n", res_init, res_uninst, res_end);

    
  if (res_init == res_uninst || res_init==res_end ){    
    printf ("OK\n");
  }else{
    printf( "KO\n");
  }
  if(res_end != res_uninst){
    printf("OK\n");
  }else{
    printf( "KO\n");
  }
  return RUNNING_ON_VALGRIND;
}
