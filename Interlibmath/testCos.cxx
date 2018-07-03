
#include<iostream> 
#include<math.h> 

int main(int argc, char** argv){

  double a(0.1);
  float af(a);

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

  
  
};
