
#include<iostream> 
#include<math.h> 

int main(int argc, char** argv){

  double a(0.1);
  float af(a);
  long double al= 1./10.;

  for(int i=0; i<4; i++){
    std::cout << "diff cos: " <<  cos(a) -cos(a) << std::endl;
    std::cout << "diff cosf: " <<  cosf(af) -cosf(af) << std::endl;
  }

  for(int i=0; i<4; i++){
    std::cout << "diff sin: " <<  sin(a) -sin(a) << std::endl;
    std::cout << "diff sinf: " <<  sinf(af) -sinf(af) << std::endl;
  }

  for(int i=0; i<4; i++){
    std::cout << "diff erf: " <<  erf(a) -erf(a) << std::endl;
    std::cout << "diff erff: " <<  erff(af) -erff(af) << std::endl;
  }

  std::cout << "sqrt: "<<sqrt(a)<<std::endl;
  //  std::cout << "sqrtf: "<<sqrtf(af)<<std::endl;
  std::cout << "sqrtl: "<<sqrtl(al)<<std::endl;
  for(int i=0; i<6; i++){
    std::cout << "diff sqrt: " <<  sqrt(a) -sqrt(a) << std::endl;
    std::cout << "diff sqrtf: " <<  sqrtf(af) -sqrtf(af) << std::endl;
    std::cout << "diff sqrtl: " <<  sqrtl(al) -sqrtl(al) << std::endl;
  }

  
  
};
