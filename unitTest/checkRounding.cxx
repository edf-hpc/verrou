
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string.h>

#include <fenv.h>
#include <limits>

#include "verrou.h"

#ifdef  TEST_FMA
#include  <immintrin.h>
#include  <fmaintrin.h>
#endif 

void usage(char** argv){
  std::cout << "usage : "<< argv[0]<< " ENV ROUNDING_MODE  avec "<<std::endl;
  std::cout << "ROUNDING_MODE in [upward, toward_zero, downward, nearest]  random, average are not valid"<<std::endl;
  std::cout << "ENV in [valgrind fenv]"<<std::endl;  
}
int roundingMode=-2;
bool fenv;

void stopInst(bool fenv, int roundingMode){
  if(fenv){
    fesetround(FE_TONEAREST);
  }else{
    VERROU_STOP_INSTRUMENTATION;
  }
}

void startInst(bool fenv, int roundingMode){
  if(fenv){
    fesetround(roundingMode);
  }else{
    VERROU_START_INSTRUMENTATION;
  }
}

template<class REALTYPE>
class test{
public:
  test(REALTYPE a):expectedResult(a){
  }
  
  REALTYPE res;
  REALTYPE expectedResult; 
  void check(){
    std::cout.precision(std::numeric_limits< REALTYPE >::digits10);
    std::cout << name()<<"<"<< typeName(res)<<">" <<":\tres: " << res
	      << "\ttheo: "<< expectedResult
	      << "\tdiff: "<<  res-expectedResult<<std::endl;
    
  }
  
  void run(){
    startInst(fenv,roundingMode);    
    res=compute();
    stopInst(fenv,roundingMode);
    check();
  }

  template<class REAL> std::string typeName(REAL& a){
    return std::string("unknown");
  }

  std::string typeName(long double& a){
    return std::string("long double");
  }

  std::string typeName(double& a){
    return std::string("double");
  }
  
  std::string  typeName(float& a){
    return std::string("float");
  }



  virtual REALTYPE compute()=0;
  virtual std::string name()=0;
};



template<class REALTYPE>
class test1: public test<REALTYPE>{
 public:
  test1():test<REALTYPE>(100001.),
	  size(1000000),
	  step(0.1),
	  init(1.)
    {
    }

  std::string name(){
    return std::string("test1");
  }

  REALTYPE compute(){    
    REALTYPE acc=init;
    for(int i=0; i<size; i++){
      acc+=step;
    }
    return acc;
  }

  
 private:
  const int size;
  const REALTYPE step;
  const REALTYPE init;
};



template<class REALTYPE>
class test2: public test<REALTYPE>{
 public:
  test2():test<REALTYPE>(10000),
	  size(1000000),
	  step(0.1),
	  init(0.)
    {

    }

  std::string name(){
    return std::string("test2");
  }


  REALTYPE compute(){    
    REALTYPE acc=init;
    for(int i=0; i<size; i++){
      acc+=step*step;
    }  
    return acc;
  }
  
 private:
  const int size;
  const REALTYPE step;
  const REALTYPE init;
};


template<class REALTYPE>
class test3:public test<REALTYPE>{
 public:
  test3():test<REALTYPE>(100000),
	  size(1000000),
	  stepDiv(10.),
	  init(0.)
    {
    }

  std::string name(){
    return std::string("test3");
  }


  REALTYPE compute(){    
    REALTYPE acc=init;
    for(int i=0; i<size; i++){
      acc+=(1/stepDiv);
    }  
    return acc;
  }

  
 private:
  const int size;
  const REALTYPE stepDiv;
  const REALTYPE init;
};





template<class REALTYPE>
class test4:public test<REALTYPE>{
 public:

  //The size are adapted to avoid inf
  static int getSize(long double a){ return 150;};
  static int getSize(double a){ return 150;};
  static int getSize(float a){ return 34;} ;
  static int getSize(){return getSize((REALTYPE)0.);};
  
  test4():test<REALTYPE>(1.),
	  size(getSize()),
	  init(1.){}

  std::string name(){
    return std::string("test4");
  }


  REALTYPE compute(){    
    REALTYPE prod=init;
    for(int i=1; i<size; i++){
      prod=prod*i;    
    } 
    for(int i=1; i<size; i++){
      prod=prod/i;
    }  

    return prod;
  }
  
 private:
  const int size;

  const REALTYPE init;
};



template<class REALTYPE>
class test5:public test<REALTYPE>{
 public:

  inline double myFma(const double& a, const double& b, const double& c){
    double d;
#ifdef TEST_FMA
    __m128d ai, bi,ci,di;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmadd_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);
#else
    d=a*b+c;
#endif
    return d;
  }


  inline float myFma(const float& a, const float& b, const float& c){
    float d;
#ifdef TEST_FMA
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    bi = _mm_load_ss(&b);
    ci = _mm_load_ss(&c);
    di=_mm_fmadd_ss(ai,bi,ci);
    d=_mm_cvtss_f32(di);
#else
    d=a*b+c;
#endif
    return d;
  }


  
  test5():test<REALTYPE>(10000),
    size(1000000),
    value(0.1),
    init(0.){}

  std::string name(){
    return std::string("test5");
  }


  REALTYPE compute(){    
    REALTYPE acc=init;

    for(int i=0; i<size; i++){
      acc=myFma(value,value,acc);
    } 
    return acc;
  }
  
 private:
  const int size;
  const REALTYPE value;  
  const REALTYPE init;

};





int main(int argc, char** argv){
  std::string roundingModeStr;
  std::string env;

  

  if(argc==3){
    env=argv[1];
    roundingModeStr=argv[2];    
  }else{
    if(argc==2){
      env=argv[1];
      roundingModeStr=std::string("unknown");
      roundingMode=-1;
    }else{
      usage(argv);
      return EXIT_FAILURE;
    }
  }

  //  std::cout << "env: "<<env<<std::endl;
  //  std::cout << "roundingMode: "<<roundingModeStr<<std::endl;
  
  //Parse ENV

  if(env==std::string("fenv")){
    fenv=true;
  }else{
    if(env==std::string("valgrind")){
      fenv=false;
    }else{
      usage(argv);
      return EXIT_FAILURE;
    }
  }
  
  
  //Parse ROUNDING_MODE
  if(roundingModeStr==std::string("upward")) roundingMode=FE_UPWARD;
  if(roundingModeStr==std::string("downward")) roundingMode=FE_DOWNWARD;
  if(roundingModeStr==std::string("nearest")) roundingMode=FE_TONEAREST;
  if(roundingModeStr==std::string("toward_zero")) roundingMode=FE_TOWARDZERO;
  
  if(roundingMode==-2){
    usage(argv); 
    return EXIT_FAILURE;
  }
  
  {
    typedef double RealType;
    test1<RealType> t1; t1.run();
    test2<RealType> t2; t2.run();
    test3<RealType> t3; t3.run();
    test4<RealType> t4; t4.run();
    test5<RealType> t5; t5.run();
    }
  
  {
    typedef float RealType;
    test1<RealType> t1; t1.run();
    test2<RealType> t2; t2.run();
    test3<RealType> t3; t3.run();
    test4<RealType> t4; t4.run();
    test5<RealType> t5; t5.run();
  }

    {
    typedef long double RealType;
    test1<RealType> t1; t1.run();
    test2<RealType> t2; t2.run();
    test3<RealType> t3; t3.run();
    test4<RealType> t4; t4.run();
    //test5<RealType> t5; t5.run();
    }


  return EXIT_SUCCESS;
}



