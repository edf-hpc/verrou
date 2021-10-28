#include "interflop_check_float_max.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>

#include <float.h>


void printFltMax(){
  std::cout << "FLT_MAX detected " << std::endl;
}


int main(int argc, char** argv){
#ifndef NATIF
  void* context;
  struct interflop_backend_interface_t ifcheck_float_max=interflop_check_float_max_init(&context);
  ifmax_set_max_handler(&printFltMax);
#endif

  double a=DBL_MAX;
  double b=1;
  double c;

  float af=FLT_MAX;
  float bf=0.10;
  float cf=af-bf;

  std::cout << "check max"<< std::endl;
#ifdef NATIF
  std::cout << "double" << std::endl;
  c=a-b;
  std::cout << "float" << std::endl;  
  cf=af-bf;
#else
  std::cout << "double" << std::endl;
  interflop_check_float_max_sub_double(a,b,&c,context);
  std::cout << "float" << std::endl;  
  interflop_check_float_max_sub_float(af,bf,&cf,context);
#endif

  std::cout << "check without max"<< std::endl;

  
#ifdef NATIF
  std::cout << "double" << std::endl;
  c=b-b;
  std::cout << "float" << std::endl;
  cf=bf-bf;
#else
  std::cout << "double" << std::endl;
  interflop_check_float_max_sub_double(b,b,&c,context);
  std::cout << "float" << std::endl;
  interflop_check_float_max_sub_float(bf,bf,&cf,context);
#endif
  
  //  std::cout << std::setprecision(16);
  //  std::cout << "c: "<<c << std::endl;

#ifndef NATIF
  interflop_check_float_max_finalize(context);
#endif
  return 0;
}
