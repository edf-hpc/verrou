
#define DEBUG_PRINT_OP


#include "interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>




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
  struct interflop_backend_interface_t ifverrou=interflop_verrou_init(&context);

  interflop_verrou_configure(VR_AVERAGE_SCOMDET, context);

  uint64_t seed=0;
  seed=1020000002;
  if(argc==2){
    seed=atoi(argv[1]);
  }

  std::cout << std::setprecision(16);
  std::cout << "seed: " << seed<<std::endl;
  //seed=1000000000;
  // if(conf.seed==(unsigned int) -1){
  //   struct timeval t1;
  //   gettimeofday(&t1, NULL);
  //   conf.seed =  t1.tv_usec + getpid();
  // }
  verrou_set_seed (seed);

  double v1,v2;
  interflop_verrou_mul_double(0.1, 0.1,&v1,context);
  interflop_verrou_mul_double(0.1,-0.1,&v2,context);
  std::cout << "v1+v2: " << v1+v2<< std::endl;

  verrou_set_debug_print_op(&print_debug);

  double step0=0.1;
  double acc0=0.;
  double step1=-0.1;
  double acc1=0.;

  for(int i=0; i<10000; i++){
    // std::cout << "i: " <<i<<std::endl;
    interflop_verrou_add_double(acc0,step0,&acc0,context);
    interflop_verrou_add_double(acc1,step1,&acc1,context);
    //    std::cout << "acc0+acc1: "<<acc0+acc1 << std::endl;
  }


  std::cout << "acc0: "<<acc0 << std::endl;
  std::cout << "acc1: "<<acc1 << std::endl;
  std::cout << "acc0+acc1: "<<acc0+acc1 << std::endl;

  interflop_verrou_finalize(context);


  return 0;
}
