
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string.h>

#include <fenv.h>
#include <limits>

#include "verrou.h"

void usage(char** argv){
  std::cout << "usage : "<< argv[0]<< " ROUNDING_MODE ENV avec "<<std::endl;
  std::cout << "ROUNDING_MODE in [UPWARD, TOWARDZERO, DOWNWARD, NEAREST]  RANDOM, AVERAGE are not valid"<<std::endl;
  std::cout << "ENV in [valgrind fenv]"<<std::endl;  
}
int roundingMode=-1;
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





int main(int argc, char** argv){
  std::string roundingModeStr;
  std::string env;
  if(argc==3){
    roundingModeStr=argv[1];
    env=argv[2];
    
  }else{
    usage(argv);
    return EXIT_FAILURE;
  }

  //Parse ENV

  if(env==std::string("FENV")){
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
  if(roundingModeStr==std::string("UPWARD")) roundingMode=FE_UPWARD;
  if(roundingModeStr==std::string("DOWNWARD")) roundingMode=FE_DOWNWARD;
  if(roundingModeStr==std::string("NEAREST")) roundingMode=FE_TONEAREST;
  if(roundingModeStr==std::string("TOWARDZERO")) roundingMode=FE_TOWARDZERO;
  
  if(roundingMode==-1){
    usage(argv); 
    return EXIT_FAILURE;
  }
  

  {
    typedef double RealType;
    test1<RealType> t1; t1.run();
    test2<RealType> t2; t2.run();
    test3<RealType> t3; t3.run();
    test4<RealType> t4; t4.run();
    }
  
  {
    typedef float RealType;
    test1<RealType> t1; t1.run();
    test2<RealType> t2; t2.run();
    test3<RealType> t3; t3.run();
    test4<RealType> t4; t4.run();
  }


  return EXIT_SUCCESS;
}



