
#define DEBUG_PRINT_OP

#include "interflop_mcaquad.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include <sys/time.h>
#include <unistd.h>

void print_debug(int nbArg, const char * op,const  double* a, const double* res){

  if(nbArg==1){
    std::cout << op << " : "<< a[0] << "->"<<&res << std::endl;  
  }

  if(nbArg==2){
    std::cout << op << " : "<< a[0] << "," << a[1]<< "->"<<&res << std::endl;  
  }

  if(nbArg==3){
    std::cout << op << " : "<< a[0] << "," << a[1]<< "," << a[2]<< "->"<<&res << std::endl;  
  }

} ;


int main(int argc, char** argv){

  void* context;
  
  struct interflop_backend_interface_t ifmcaquad=interflop_mcaquad_init(&context);

  //  interflop_verrou_configure(VR_NEAREST, context);
  mcaquad_conf_t conf;
  conf.precision_float=32;
  conf.precision_double=53;
  conf.precision_double=20;
  conf.mode=MCAMODE_MCA;
  //MCAMODE_PB;
  //MCAMODE_RR;
  interflop_mcaquad_configure(conf, context);

  struct timeval now;
  gettimeofday(&now, NULL);
  unsigned int pid = getpid();
  unsigned int firstSeed = now.tv_usec + pid;

  firstSeed=3;
  mcaquad_set_seed(firstSeed);

  //verrou_set_debug_print_op(&print_debug);
  

  double a=0.1;
  double b=0.1;
  double c1;
  double c2;

  interflop_mcaquad_add_double(a,b,&c1,context);
  ifmcaquad.interflop_add_double(a,b,&c2,context);
  
  std::cout << std::setprecision(16);
  std::cout << "c1: "<<c1 << std::endl; 
  std::cout << "c2: "<<c2 << std::endl; 

  ifmcaquad.interflop_madd_double(a,b,c1,&c2,context);

  std::cout << "fma: "<<c2 << std::endl;

  //  ifmcaquad.interflop_madd_double(a,b,c1,&c2,context)
  
  interflop_mcaquad_finalyze(context);
  

  return 0;
}


