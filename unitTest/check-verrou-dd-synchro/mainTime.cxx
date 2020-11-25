#include <iostream>

#ifdef USE_VERROU_SYNCHRO
#include "valgrind/verrouSynchroLib.h"
#else
#define VERROU_SYNCHRO_INIT
#define VERROU_SYNCHRO(a,b)
#define VERROU_SYNCHRO_FINALIZE
#endif





//exemple suite de muller
int main(int argc, char** argv){
  VERROU_SYNCHRO_INIT;
  
  int nt=12;

  VERROU_SYNCHRO("Init", 0);
  double x0 = 11./2.;
  double x1 = 61./11.;


  for(int it =0; it <nt ; it++){
    VERROU_SYNCHRO("Time Loop", it);

    double x2 = 111. - (1130. - 3000./x0) / x1;
    x0 = x1;
    x1 = x2;
  }
  
  VERROU_SYNCHRO("Fin", 0);
  std::cout << "x1["<< nt<< "]: " << x1<<std::endl;

  VERROU_SYNCHRO_FINALIZE;
};
