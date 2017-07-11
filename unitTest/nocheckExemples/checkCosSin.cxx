
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string.h>


#include <limits>

#include "verrou.h"

//VERROU_STOP_INSTRUMENTATION;
//VERROU_START_INSTRUMENTATION;


void verrou_stop(const void* constPtr)    {
  volatile void* ptr=const_cast<void*>(constPtr);
  __asm__ volatile (" ":: "X" (ptr) :"memory"  );
  VERROU_STOP_INSTRUMENTATION;  
  __asm__ volatile (" ":: "X" (ptr) :"memory"  );
}

void verrou_start(const void* constPtr)    {
  volatile void* ptr=const_cast<void*>(constPtr);
  __asm__ volatile (" ":: "X" (ptr) :"memory"  );
  VERROU_START_INSTRUMENTATION;  
  __asm__ volatile (" ":: "X" (ptr) :"memory"  );
}



template<class REALTYPE>
void computeCos(int nb){
  const REALTYPE step= M_PI / (REALTYPE)(nb);
  REALTYPE accSin(0.);
  REALTYPE accCos(0.);
  std::cout << "nb:" <<nb<<std::endl;
  for(int i=2700; i<nb; i++){
    const REALTYPE x(i*step);
    //    std::cout << "x: " << x<<std::endl;
    //    verrou_stop(&x);
    REALTYPE cosx(cos(x));
    REALTYPE sinx(sin(x));
    //    verrou_start(&cosx);
    //    sinx=FORCE_EVAL_DOUBLE(sinx);
    //    cosx=FORCE_EVAL_DOUBLE(cosx);

    
    accSin+=sinx;
    accCos+=cosx;
    
    if((cosx > 1.)  ||  (cosx < -1.) ) {
      std::cout << "cosx:" << cosx<< " i:"<<i<<std::endl;
      std::cout << "sinx:" << sinx<< " i:"<<i<<std::endl;
      exit(42);
    }    
    if((sinx > 1.)|| (sinx < -1.)) {
      std::cout << "cosx:" << cosx<< " si:"<<i<<std::endl;;
      std::cout << "sinx:" << sinx<< " si:"<<i<<std::endl;;
      exit(42);
    }

    
  }
  std::cout << "accCos:" <<accCos<<std::endl;
  std::cout << "accSin:" <<accSin<<std::endl;
};



int main(int argc, char** argv){
  int nb=10000;
  std::cout << " computeCos<double>"<<std::endl;
  computeCos<double>(nb);
  
  //  std::cout << " computeCos<float>"<<std::endl;
  //  computeCos<float>(nb);

  return EXIT_SUCCESS;
}


