#include <iostream>
#include <iomanip>
#include <cmath>

typedef double Realtype;

Realtype sqrtNewton(Realtype a, size_t nbMax=100){
  Realtype xp=0;
  Realtype xn=a;
  size_t i=0;
  bool first=true;
  while(true){
    xp=xn;
    xn=1/2.*(xp+a/xp);
    i+=1;
    std::cout << "xn["<<i<<"]="<< xn <<"  delta="<<xn-xp<< std::endl;
    if(xn==xp){
      if(!first){
	return xn;
      }
      first=false;
    }else{
      first=true;
    }
    if(i==nbMax){
      std::cout << "nbItMax reached"<<std::endl;
      return xn;
    }
  }
}


        
int main(int argc, char** argv){
  std::cout << std::setprecision(17);
  Realtype res=sqrtNewton(0.1);
  std::cout << "res=" << res<<std::endl;
};
