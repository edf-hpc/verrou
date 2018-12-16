
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#include <float.h>
#include <quadmath.h>
#include "../backend_verrou/interflop_verrou.h"
#include "../backend_verrou/vr_rand.h"
#include "../backend_verrou/vr_roundingOp.hxx"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>

#define MAGIC(name) gugtd1az1dza ## name




vr_RoundingMode ROUNDINGMODE;
void (*vr_cancellationHandler)(int)=NULL;
void (*vr_panicHandler)(const char*)=NULL;
void (*vr_nanHandler)()=NULL;


 void __attribute__((constructor)) init_interlibmath(){
   
   struct timeval now;
   gettimeofday(&now, NULL);
   unsigned int pid = getpid();
   unsigned int vr_seed=  now.tv_usec + pid;
   vr_rand_setSeed(&vr_rand, vr_seed);

   const char* vrm=std::getenv("VERROU_ROUNDING_MODE");

   if(vrm!=NULL){
     std::string envString(vrm);
     if(envString==std::string("random")){
       ROUNDINGMODE=VR_RANDOM;
     }
     if(envString==std::string("average")){
       ROUNDINGMODE=VR_AVERAGE;
     }
     if(envString==std::string("nearest")){
       ROUNDINGMODE=VR_NEAREST;
     }
     if(envString==std::string("upward")){
       ROUNDINGMODE=VR_UPWARD;
     }
     if(envString==std::string("downward")){
       ROUNDINGMODE=VR_DOWNWARD;
     }
     if(envString==std::string("toward_zero")){
       ROUNDINGMODE=VR_ZERO;
     }
     if(envString==std::string("farthest")){
       ROUNDINGMODE=VR_FARTHEST;
     }
     if(envString==std::string("float")){
       ROUNDINGMODE=VR_FLOAT;
     }
   }
}


void __attribute__((destructor)) finalyze_interlibmath(){

};



class myLibMathFunction1{
public:
  myLibMathFunction1(std::string name){
    load_real_sym((void**)&(real_name_float) , name +std::string("f"));
    load_real_sym((void**)&(real_name_double) , name);
    load_real_sym((void**)&(real_name_float128) , name +std::string("q"));
  }
  
  double apply(double a){
    return real_name_double(a);
  }

  float apply(float a){
    return real_name_float(a);
  }

  __float128 apply(__float128 a){
    return real_name_float128(a);
  }


private:
  void load_real_sym(void**fctPtr, std::string name ){
    // std::cerr << "loading: " <<  name <<std::endl;
    (*fctPtr) =dlsym(RTLD_NEXT, name.c_str());
    if(*fctPtr==NULL){
      std::cerr << "Problem with function "<< name<<std::endl;
    }
  }

  //Attributs
  float (*real_name_float)(float) ;
  double (*real_name_double)(double) ;
  __float128 (*real_name_float128)(__float128) ;
};


//myLibMathFunction1 myCos("cos");
enum FunctionName {enumCos, enumSin, enumErf, enumSqrt, size};

myLibMathFunction1 functionNameTab[size]={myLibMathFunction1("cos"),
					  myLibMathFunction1("sin"),
					  myLibMathFunction1("erf"),
					  myLibMathFunction1("sqrt")
};


template<int  MATHFUNCTIONINDEX, typename REALTYPE>
class libMathFunction{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  

#ifdef DEBUG_PRINT_OP
  static const char* OpName(){return "libmath ?";}
#endif

  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    return functionNameTab[MATHFUNCTIONINDEX].apply(a);
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
    const RealType & a(p.arg1);
    __float128 ref=functionNameTab[MATHFUNCTIONINDEX].apply((__float128)a);
    const __float128 error128=  ref -(__float128)z ;
    return (RealType)error128;
  };

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return error(p,c) ;
  };

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }


  static inline void check(const PackArgs& p, const RealType& d){
  };

};





template<class REALTYPE> 
REALTYPE MAGIC(constraint_m1p1)(const REALTYPE& x ){
  if(x>1) return 1.; 
  if(x<-1) return -1.;
  return x;
}



extern "C"{


  double cos(double a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumCos,double>,double > Op;
    double res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res); // not sur it is usefull
  }

  float cosf(float a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumCos,float>,float > Op;
    float res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res);
  }


  double sin(double a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumSin,double>,double > Op;
    double res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res); // not sur it is usefull
  }

  float sinf(float a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumSin,float>,float > Op;
    float res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res);
  }


  double erf(double a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumErf,double>,double > Op;
    double res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res); // not sur it is usefull
  }

  float erff(float a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumErf,float>,float > Op;
    float res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return MAGIC(constraint_m1p1)(res);
  }


  double sqrt(double a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumSqrt,double>,double > Op;
    double res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return res;
  }

  float sqrtf(float a){
    typedef OpWithSelectedRoundingMode<libMathFunction<enumSqrt,float>,float > Op;
    float res;
    Op::apply(Op::PackArgs(a) ,&res,NULL);
    return res;
  }

  
  
  
};

