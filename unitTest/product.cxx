
#include <iostream>
#include "vr_DekkerOps.h"
#include <cstdlib>
#include <math.h>

template<class REALTYPE> 
REALTYPE nextAfter(REALTYPE a){
  //std::cout <<"Problem"<<std::endl;
  exit(42);
};

template<> 
double nextAfter<double>(double a){
  double res;
  if(a>=0){
    res=nextafter(a,2*a+1);
  }else{
    res=nextafter(a,0);
  }
  return res;
};


template<> 
float nextAfter<float>(float a){
  float res;
  if(a>=0){
    res=nextafterf(a,2*a+1);
  }else{
    res=nextafterf(a,0);
  }
  return res;
};



template<class REALTYPE> 
REALTYPE nextPrev(REALTYPE a){
  //std::cout <<"Problem"<<std::endl;
  exit(42);
};

template<> 
double nextPrev<double>(double a){
  double res;
  if(a>=0){
    res=nextafter(a,0);
  }else{
    res=nextafter(a,2*a-1);
  }
  return res;
};


template<> 
float nextPrev<float>(float a){
  float res;
  if(a>=0){
    res=nextafterf(a,0    );
  }else{
    res=nextafterf(a,2*a-1);
  }
  return res;
};






template<class REALTYPE>
bool checkProd(REALTYPE a, REALTYPE b){
  bool res=true;
  const REALTYPE expectedRes=a*b;

  REALTYPE z,zz;
  DekkerOp<REALTYPE>::twoProd(a,b,z,zz);
  //  DekkerOp<REALTYPE>::mul12(a,b,z,zz);
  REALTYPE w,ww;
  DekkerOp<REALTYPE>::twoSum(z,zz,w,ww);

  if(z!=expectedRes){
    std::cout << "KO z!= a.b" <<std::endl;
    res=false;
  }
  if(z!=w){
     std::cout << "KO z!=w" <<std::endl;
     res=false;
  }
  
  if(res){
    std::cout << "OK nearest: a: "<<a<<"\tb: "<< b<<std::endl;
    return res;
  }else{
    const REALTYPE uz = ulp(z);
    const REALTYPE ures = ulp(expectedRes);

    REALTYPE uzp=(nextAfter  (z)-z);
    REALTYPE uzm=z-(nextPrev  (z));

    std::cout << "KO: a: " << a<< "\tb: " << b
	      << "\tz: "<< z
	      << "\texpectedRes: "<< expectedRes << "\t z-expected:" << z-expectedRes
      	      << "\tzz: "<< zz
	      << "\tulpz: "<<uz<< "\tulpres:"<< ures
	      << "\tzz / ulp: "<< (zz/ uz)
	      <<std::endl;  

    
    if(uzp!=uz){
      std::cout << "Problem ulp: "<< uz<<"a: " << a<< "\tb: " << b  <<std::endl;        
      return false;    
    }
    

  }
  return true;
}


int main(int argc, char** argv){
  float a,b;
  if(argc==3){
    a=atof(argv[1]);
    b=atof(argv[2]);
  }

  checkProd(a,b);
    

  a=0.1,b=10;
  checkProd(a,b);


  a=0.1,b=0.1;
  checkProd(a,b);

  a=1,b=1;
  checkProd(a,b);

  a=1.1,b=.1;
  checkProd(a,b);


  a=1.00000000000001,b=.1;
  checkProd(a,b);




  return EXIT_SUCCESS;
}
