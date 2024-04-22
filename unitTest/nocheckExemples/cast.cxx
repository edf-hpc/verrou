#include <iostream>
#include <math.h>
#include <cstdlib>


//#include  <immintrin.h>
//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  double a;
  if(argc==2){
    a=atof(argv[1]);
  }else{
    std::cerr << "demande 1 argument"<<std::endl;
    return EXIT_FAILURE;
  }

  float af=(float)a;

  std::cout << "a:" <<a<<std::endl;
  std::cout << "af:" <<af<<std::endl;
  std::cout << "diff:" <<a -(double)af<<std::endl;

  return EXIT_SUCCESS;
}
