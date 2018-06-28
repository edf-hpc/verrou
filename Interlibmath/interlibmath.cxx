
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#include <float.h>
#include <quadmath.h>
#include "../backend_verrou/vr_rand.h"
#include "../backend_verrou/vr_roundingOp.hxx"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

#define MAGIC(name) gugtd1az1dza ## name

#include <dlfcn.h>

typedef struct
{
  double (*real_cos_double)(double) ;
  double (*real_sin_double)(double) ;
  
} libmath_handler_t;

static libmath_handler_t libmath_handler ;

void load_real_sym(void**fctPtr, std::string name ){
  (*fctPtr) =dlsym(RTLD_NEXT, name.c_str());
}

 void __attribute__((constructor)) init_interlibmath(){
   

   struct timeval now;
   gettimeofday(&now, NULL);
   unsigned int pid = getpid();
   unsigned int vr_seed=  now.tv_usec + pid;
   vr_rand_setSeed(&vr_rand, vr_seed);
   //std::cerr <<"Init interlibmath" <<std::endl;
   load_real_sym((void**)&(libmath_handler.real_cos_double) , "cos");
   load_real_sym((void**)&(libmath_handler.real_sin_double) , "sin");

}


void __attribute__((destructor)) finalyze_interlibmath(){

};





template<typename REALTYPE>
class libmathcos{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  

#ifdef DEBUG_PRINT_OP
  static const char* OpName(){return "libmathcos";}
#endif

  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    double res=(*libmath_handler.real_cos_double)(a);
    return res;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
    const RealType & a(p.arg1);
    const __float128 ref=cosq((__float128)a ); 
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
REALTYPE MAGIC(constraint_cossin)(const REALTYPE& x ){
  if(x>1) return 1.; 
  if(x<-1) return -1.;
  return x;
}




template<class OP>
class OpWithSelectedRoundingMode<OP,typename OP::RealType>{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;
  
  static inline RealType apply(const PackArgs& p){    
    // return RoundingNearest<OP>::apply (p);
    //  return RoundingUpward<OP>::apply (p);
    //  return RoundingDownward<OP>::apply (p);
    //  return RoundingZero<OP>::apply (p);
    return RoundingRandom<OP>::apply (p);
    //return RoundingAverage<OP>::apply (p);
    //  return RoundingFarthest<OP>::apply (p);
  }
};


extern "C"{
  //double cos(double a);
  


  double cos(double a){
    typedef OpWithSelectedRoundingMode< libmathcos<double>,double > Op;
    double res=Op::apply((Op::PackArgs(a) ));
    return MAGIC(constraint_cossin)(res);
  }
};

