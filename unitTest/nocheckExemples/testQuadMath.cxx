
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <iomanip>
#ifdef __x86_64__
#include  <quadmath.h>
#endif

#ifdef __aarch64__
#endif


//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  double remain;
  
#ifdef __x86_64__
  __float128 a=1.;
  __float128 quarterPi= atanq(a);
  __float128 pi= quarterPi * __float128(4.);
  __float128 factor=0.1;
  remain= (double)( (pi - (__float128)M_PI)*factor);
#endif

#ifdef __aarch64__
  using myReal128=long double ;
  myReal128 a=1.;
  myReal128 quarterPi= atanl(a);
  myReal128 pi= quarterPi * myReal128(4.);
  myReal128 factor=0.1;
  remain= (double)( (pi - (myReal128)M_PI) *factor);
  
#endif
  std::cout << std::setprecision(17);
  std::cout << "remain: "<< remain<<std::endl;

  return EXIT_SUCCESS;
}
