#include<iostream>
#include<iomanip>
#include<cstdlib>

#include <cfloat>
#include <cmath>


#ifdef WITH_REFERENCE
#include <boost/multiprecision/mpfi.hpp> 
using namespace boost::multiprecision;
typedef number<mpfi_float_backend<1000> > mpfi_realtype;

namespace std{
  mpfi_realtype log(mpfi_realtype& a){
    return boost::multiprecision::log(a);
  };
};

template<class FUNCTOR>
struct computeError{
  
 
  template<class REALTYPE >
  static mpfi_realtype apply(const REALTYPE& a1, const REALTYPE& a2){
    mpfi_realtype interA1(a1);
    mpfi_realtype interA2(a2);
    REALTYPE resIEEE= FUNCTOR::apply(a1,a2);
    mpfi_realtype resIEEE_MPFI(resIEEE);
    mpfi_realtype resMPFI=FUNCTOR::apply(interA1, interA2);
    
    mpfi_realtype nbBit= -log(abs((resIEEE_MPFI - resMPFI) / resMPFI))/log(mpfi_realtype(2.) );
    return nbBit;
  }
};

#endif


struct areaInstable{
  template<class REALTYPE>
  static REALTYPE apply(REALTYPE a1, REALTYPE a2){
    if(a1==a2){
      return a1;
    }else{
      return (a2-a1) / (std::log(a2) -std::log(a1));
    }
  }
};

struct areaCorrected{
  template<class REALTYPE>
  static REALTYPE apply(REALTYPE a1, REALTYPE a2){
    if(a1==a2){
      return a1;
    }else{
      REALTYPE x=a2/a1;
      return a1*(x-1) / (std::log(x));
    }
  }
};


int main(int argc, char** argv){
  int numberSample=1;
  bool ref=false;

  if(argc==2){
    numberSample=atoi(argv[1]);   
  }
  if(numberSample==-1){
    ref=true;
    numberSample=0;
  }
  

  double a= 4.2080034963016440E-005;
  float af= 4.2080034963016440E-005;
  int numberEpsilon=3;

  double a1double= a, a2double=a+ numberEpsilon* DBL_EPSILON ;
  float a1float= af, a2float=af+ numberEpsilon* FLT_EPSILON ;
  if(!ref){
    std::cout << "BeforeCorrection_Double"
	      << "\t" << "AfterCorrection_Double"
	      << "\t" << "BeforeCorrection_Float"
	      << "\t" << "AfterCorrection_Float"<<std::endl;
  }
  std::cout << std::setprecision(42);
  for(int i=0; i< numberSample ; i++){

    double resInstabledouble  = areaInstable::apply<double> (a1double, a2double);
    double resCorrecteddouble = areaCorrected::apply<double> (a1double, a2double);

    float resInstablefloat  = areaInstable::apply<float> (a1float, a2float);
    float resCorrectedfloat = areaCorrected::apply<float> (a1float, a2float);

    //std::cout << std::setprecision(16);
    std::cout << resInstabledouble << "\t"<<  resCorrecteddouble<<"\t";
    //    std::cout << std::setprecision(8);
    std::cout << resInstablefloat << "\t"<<  resCorrectedfloat<<std::endl;
      
  }
#ifdef WITH_REFERENCE
  if(ref){
    std::cout << "Double Before : " << computeError<areaInstable>::apply  (a1double,a2double)  <<std::endl;
    std::cout << "Double After : "  << computeError<areaCorrected>::apply (a1double,a2double)  <<std::endl;
    std::cout << "Float Before : "  << computeError<areaInstable>::apply  (a1float,a2float)  <<std::endl;
    std::cout << "Float After : "   << computeError<areaCorrected>::apply (a1float,a2float)  <<std::endl;  
  }
#endif
}
