#include <iostream>
#include <iomanip>

#include <cmath>


int main(int argc, char** argv){

  std::cout << std::setprecision(17);

  std::cout << "cos(0.1): "<<cos(0.1) <<std::endl;
  std::cout << "sinf(0.1): "<<sinf(0.1) <<std::endl;
  std::cout << "erf(0.01): "<<erf(0.01) <<std::endl;    
  std::cout << "atan2(0.1,0.01): "<<atan2(0.1, 0.01) <<std::endl;    
  std::cout << "jn(2,0.01): "<<jn(2, 0.01) <<std::endl;    
  
  return EXIT_SUCCESS;
}
