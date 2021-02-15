
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "interflop_checkdenorm.h"

void printDenorm(){
  std::cout << "Denorm deteted " << std::endl;
}

void oups(const char* msg){
  std::cout << "oups:  " << msg<<std::endl;
}

int main(int argc, char** argv){

  void* context;
  struct interflop_backend_interface_t ifcheckdenorm=interflop_checkdenorm_init(&context);

  checkdenorm_conf conf;
  conf.flushtozero=true;
  interflop_checkdenorm_configure(conf, context);

  checkdenorm_set_denorm_handler(&printDenorm);
  checkdenorm_set_panic_handler(&oups);

  
  double a=1.e-160;
  double b=1.e-160;
  double c=1;//=a*b;

  float af=1.e-20;
  float bf=-1.e-20;
  float cf=1;//=af*bf;

  double a1=1.e-320;
  double a2=-1.;
  double a3=-1.1e-320;
  double cfma=1.;

  
  interflop_checkdenorm_mul_double(a,b,&c,context);
  interflop_checkdenorm_mul_float(af,bf,&cf,context);

  interflop_checkdenorm_madd_double(a1,a2,a3, &cfma,context);

  double adouble=0.1;
  float resFloat=1.;
  interflop_checkdenorm_cast_double_to_float(adouble, &resFloat,context);

  std::cout << std::setprecision(16);
  std::cout << "c: "<<c << std::endl;
  std::cout << "c: "<<cf << std::endl;
  std::cout << "cfma: "<<cfma << std::endl;

  std::cout << "cast: "<< resFloat<<std::endl;
  
  interflop_checkdenorm_finalyze(context);

  return 0;
}
