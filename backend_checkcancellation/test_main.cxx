#include "interflop_checkcancellation.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>



void printCancellation(int range){
  std::cout << "Cancellation deteted : "<<range << std::endl;
}

int main(int argc, char** argv){

  void* context;
  struct interflop_backend_interface_t ifcheckcancellation=interflop_checkcancellation_init(&context);

  //  interflop_checkcancellation_configure(VR_NEAREST, context);
  checkcancellation_conf conf;
  conf.threshold_float=15;
  conf.threshold_double=40;
  interflop_checkcancellation_configure(conf, context);

  checkcancellation_set_cancellation_handler(&printCancellation);

  double a=0.1000000000001;
  double b=0.10;
  double c=a-b;

  float af=0.101;
  float bf=0.10;
  float cf=af-bf;


  interflop_checkcancellation_sub_double(a,b,&c,context);
  interflop_checkcancellation_sub_float(af,bf,&cf,context);

  std::cout << std::setprecision(16);
  std::cout << "c: "<<c << std::endl;

  interflop_checkcancellation_finalyze(context);

  return 0;
}
