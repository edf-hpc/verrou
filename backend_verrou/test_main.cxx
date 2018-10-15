
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

  //  interflop_verrou_configure(VR_NEAREST, context);
  interflop_verrou_configure(VR_FLOAT, context);

  verrou_set_debug_print_op(&print_debug);
  

  double a=0.1;
  double b=0.1;
  double c;
  
  interflop_verrou_add_double(a,b,&c,context);

  std::cout << std::setprecision(16);
  std::cout << "c: "<<c << std::endl; 


  
  interflop_verrou_finalyze(context);
  

  return 0;
}


