#include <iostream>

#ifdef USE_VERROU_TASK
#include "valgrind/libverrouTask.h"
#else
#define VERROU_TASK_INIT
#define VERROU_TASK(a,b)
#define VERROU_TASK_FINALIZE
#endif





//exemple suite de muller
int main(int argc, char** argv){
  VERROU_TASK_INIT;
  
  int nt=12;

  VERROU_TASK("Init", 0);
  double x0 = 11./2.;
  double x1 = 61./11.;


  for(int it =0; it <nt ; it++){
    VERROU_TASK("Time Loop", it);

    double x2 = 111. - (1130. - 3000./x0) / x1;
    x0 = x1;
    x1 = x2;
  }
  
  VERROU_TASK("Fin", 0);
  std::cout << "x1["<< nt<< "]: " << x1<<std::endl;

  VERROU_TASK_FINALIZE;
};
