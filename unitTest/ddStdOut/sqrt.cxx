#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>

typedef double Realtype;

template<class FLUX>
Realtype sqrtNewton(Realtype a, size_t nbMax, FLUX& flux){
  Realtype xp=0;
  Realtype xn=a;
  size_t i=0;
  bool first=true;
  while(true){
    xp=xn;
    xn=1/2.*(xp+a/xp);
    i+=1;
    flux << "xn["<<i<<"]="<< xn <<"  delta="<<xn-xp<< std::endl;
    if(xn==xp){
      if(!first){
	return xn;
      }
      first=false;
    }else{
      first=true;
    }
    if(i==nbMax){
      flux << "nbItMax reached"<<std::endl;
      return xn;
    }
  }
}


int main(int argc, char** argv){
  if(argc>1){
    std::ofstream flux;
    flux.open(argv[1], std::ios::out);
    flux << std::setprecision(17);
    Realtype res=sqrtNewton(0.1, 100,flux);
    flux << "res=" << res<<std::endl;
  }else{
    std::cout << std::setprecision(17);
    Realtype res=sqrtNewton(0.1, 100,std::cout);
    std::cout << "res=" << res<<std::endl;
  }

};
