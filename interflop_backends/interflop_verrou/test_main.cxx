
#define DEBUG_PRINT_OP

#include "interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void print_debug(int nbArg, const char * op,const  double* a, const double* res){
  std::cout << std::setprecision(18);
  if(nbArg==1){
    std::cout << op << " : "<< a[0] << "->"<<res << "(" <<*res<<")"<<std::endl;
  }

  if(nbArg==2){
    std::cout << op << " : "<< a[0] << "," << a[1]<< "->"<<res << "(" <<*res<<")"<<std::endl;
  }

  if(nbArg==3){
    std::cout << op << " : "<< a[0] << "," << a[1]<< "," << a[2]<< "->"<<res << "(" <<*res<<")"<<std::endl;
  }
  std::cout << std::endl;
} ;


int main(int argc, char** argv){
  void* context;
  //  struct interflop_backend_interface_t ifverrou=interflop_init(argc,argv, &context);
  interflop_init(argc,argv, &context);

  std::cout << std::setprecision(16);

  // double v1,v2;
  // interflop_verrou_mul_double(0.1, 0.1,&v1,context);
  // interflop_verrou_mul_double(0.1,-0.1,&v2,context);
  // std::cout << "v1+v2: " << v1+v2<< std::endl;

  verrou_set_debug_print_op(&print_debug);

  const double step0=0.1;
  double acc0=1.;
  const double step1=-0.1;
  double acc1=-1.;

  float acc0f=acc0;
  float acc1f=acc1;

  double accFma0=0.;
  double accFma1=0.;

  float accFma0f=0.;
  float accFma1f=0.;

  for(int i=0; i< 1000000; i++){
    double square0, square1;
    interflop_verrou_mul_double(step0,step0,&square0,context);
    interflop_verrou_mul_double(step0,step1,&square1,context);


    interflop_verrou_add_double(square0,acc0,&acc0,context);
    interflop_verrou_add_double(square1,acc1,&acc1,context);

    float square0f, square1f;
    interflop_verrou_mul_float((float)step0,(float)step0,&square0f,context);
    interflop_verrou_mul_float((float)step1,(float)step0,&square1f,context);

    interflop_verrou_add_float(square0f,acc0f,&acc0f,context);
    interflop_verrou_add_float(square1f,acc1f,&acc1f,context);

    interflop_verrou_madd_double(step0, step0, accFma0, &accFma0,context);
    interflop_verrou_madd_double(step0, step1, accFma1, &accFma1,context);

    interflop_verrou_madd_float((float)step0, (float)step0, accFma0f, &accFma0f,context);
    interflop_verrou_madd_float((float)step1, -(float)step1, accFma1f, &accFma1f,context);
  }


  std::cout << "sum double acc0: "<<acc0 << std::endl;
  std::cout << "sum double acc1: "<<acc1 << std::endl;
  std::cout << "sum double acc0+acc1: "<<acc0+acc1 << std::endl;
  std::cout<< std::endl;

  std::cout << "sum float acc0: "<<acc0f << std::endl;
  std::cout << "sum float acc1: "<<acc1f << std::endl;
  std::cout << "sum float acc0+acc1: "<<acc0f+acc1f << std::endl;
  std::cout<< std::endl;

  std::cout << "fma double acc0: "<<accFma0 << std::endl;
  std::cout << "fma double acc1: "<<accFma1 << std::endl;
  std::cout << "fma double acc0+acc1: "<<accFma0+accFma1 << std::endl;

  std::cout<< std::endl;

  std::cout << "fma float acc0: "<<accFma0f << std::endl;
  std::cout << "fma float acc1: "<<accFma1f << std::endl;
  std::cout << "fma float acc0+acc1: "<<accFma0f+accFma1f << std::endl;

  interflop_verrou_finalize(context);


  return 0;
}
