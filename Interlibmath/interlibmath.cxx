
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

//#define MAGIC(name) gugtd1az1dza ## name


vr_RoundingMode ROUNDINGMODE;
void (*vr_cancellationHandler)(int)=NULL;
void (*vr_panicHandler)(const char*)=NULL;
void (*vr_nanHandler)()=NULL;

unsigned int my_pid;


class myLibMathFunction1{
public:
  myLibMathFunction1(std::string name):name_(name){
    load_real_sym((void**)&(real_name_float) , name +std::string("f"));
    load_real_sym((void**)&(real_name_double) , name);
    load_real_sym((void**)&(real_name_long_double) , name +std::string("l"));
    load_real_sym((void**)&(real_name_float128) , name +std::string("q"));
  }
  
  inline double apply(double a)const{
    return real_name_double(a);
  }

  inline long double apply(long double a)const{
    return real_name_long_double(a);
  }

  inline float apply(float a)const{
    return real_name_float(a);
  }

  inline __float128 apply(__float128 a)const{
    return real_name_float128(a);
  }

  const std::string& name()const{
    return name_;
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
  long double (*real_name_long_double)(long double) ;
  __float128 (*real_name_float128)(__float128) ;
  std::string name_;
};



enum FunctionName {enumcos, enumsin, enumerf, enumsqrt,
		   enumlog, enumexp, enumtan, enumasin,
		   enumacos, enumatan,
		   enum_libm_function_name_size};

myLibMathFunction1 functionNameTab[enum_libm_function_name_size]={myLibMathFunction1("cos"),
								  myLibMathFunction1("sin"),
								  myLibMathFunction1("erf"),
								  myLibMathFunction1("sqrt"),
								  myLibMathFunction1("log"),
								  myLibMathFunction1("exp"),
								  myLibMathFunction1("tan"),
								  myLibMathFunction1("asin"),
								  myLibMathFunction1("acos"),
								  myLibMathFunction1("atan")};

unsigned int libMathCounter[enum_libm_function_name_size][3][2];
void initLibMathCounter(){
  for(int i=0; i< enum_libm_function_name_size;i++){
    for(int j=0; j< 3; j++){
      libMathCounter[i][j][0]=0;
      libMathCounter[i][j][1]=0;
    }
  }
}

template<class>
struct realTypeIndex;
template<>
struct realTypeIndex<float>{
  static const int index=0;
};
template<>
struct realTypeIndex<double>{
  static const int index=1;
};
template<>
struct realTypeIndex<long double>{
  static const int index=2;
};


template<class REALTYPE, int ENUM_LIBM, int INST>
inline void incCounter(){
  libMathCounter[ENUM_LIBM][realTypeIndex<REALTYPE>::index][INST]++;
}




void printCounter(){
  std::cerr << "=="<<my_pid<<"== " << "Interlibm counter " <<std::endl;
  std::cerr << "=="<<my_pid<<"== " << "\t\t Total \tInstrumented" <<std::endl;
  for(int i=0; i< enum_libm_function_name_size;i++){

    std::cerr << "=="<<my_pid<<"== ";
    std::cerr<<  "---------------------------------------------------"<<std::endl;

    std::cerr << "=="<<my_pid<<"== ";
    std::cerr<< functionNameTab[i].name();
    int total=0;
    int totalInst=0;
    for(int j=0;j<3;j++){
      total+=libMathCounter[i][j][0]+libMathCounter[i][j][1];
      totalInst+=libMathCounter[i][j][0];
    }

    std::cerr<< "\t\t" <<  total << "\t" << totalInst<<std::endl;
    if(total!=0){
      std::cerr << "=="<<my_pid<<"== ";
      std::cerr<< " `-" " flt ";
      std::cerr<< "\t" <<  libMathCounter[i][0][0]+libMathCounter[i][0][1]  << "\t" << libMathCounter[i][0][0]<<std::endl;

      std::cerr << "=="<<my_pid<<"== ";
      std::cerr<< " `-" " dbl ";
      std::cerr<< "\t" <<  libMathCounter[i][1][0]+libMathCounter[i][1][1]  << "\t" << libMathCounter[i][1][0]<<std::endl;

      std::cerr << "=="<<my_pid<<"== ";
      std::cerr<< " `-" " lgd ";
      std::cerr<< "\t" <<  libMathCounter[i][2][0]+libMathCounter[i][2][1]  << "\t" << libMathCounter[i][2][0]<<std::endl;
    }

  }
}


template<int  MATHFUNCTIONINDEX, typename REALTYPE>
class libMathFunction{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  

#ifdef DEBUG_PRINT_OP
  static const char* OpName(){return "libmath ?";}
#endif

  static inline RealType nativeOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    return functionNameTab[MATHFUNCTIONINDEX].apply(a);
  };

  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    __float128 ref=functionNameTab[MATHFUNCTIONINDEX].apply((__float128)a);
    return (RealType)ref;
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



// template<class REALTYPE>
// REALTYPE MAGIC(constraint_m1p1)(const REALTYPE& x ){
//   if(x>1) return 1.;
//   if(x<-1) return -1.;
//   return x;
// }

extern "C"{

#define DEFINE_INTERP_LIBM_C_IMPL(FCT)\
  double FCT (double a){\
    incCounter<double, enum##FCT ,0>();\
    typedef OpWithSelectedRoundingMode<libMathFunction<enum##FCT,double>,double > Op;\
    double res;\
    Op::apply(Op::PackArgs(a) ,&res,NULL);\
    return res;\
  }\
  \
  float FCT##f (float a){\
    incCounter<float, enum##FCT,0>();\
    typedef OpWithSelectedRoundingMode<libMathFunction<enum##FCT,float>,float > Op;\
    float res;\
    Op::apply(Op::PackArgs(a) ,&res,NULL);\
    return res;\
  }\
  \
  long double FCT##l (long double a){\
    incCounter<long double, enum##FCT,1>();\
    typedef libMathFunction<enum##FCT,long double> lm;\
    return lm::nativeOp(a) ;			      \
  }

  DEFINE_INTERP_LIBM_C_IMPL(cos);
  DEFINE_INTERP_LIBM_C_IMPL(sin);
  DEFINE_INTERP_LIBM_C_IMPL(erf);
  DEFINE_INTERP_LIBM_C_IMPL(sqrt);
  DEFINE_INTERP_LIBM_C_IMPL(log);
  DEFINE_INTERP_LIBM_C_IMPL(exp);
  DEFINE_INTERP_LIBM_C_IMPL(tan);
  DEFINE_INTERP_LIBM_C_IMPL(asin);
  DEFINE_INTERP_LIBM_C_IMPL(acos);
  DEFINE_INTERP_LIBM_C_IMPL(atan);

#undef DEFINE_INTERP_LIBM_C_IMPL

};

void __attribute__((constructor)) init_interlibmath(){
  struct timeval now;
  gettimeofday(&now, NULL);
  my_pid = getpid();
  unsigned int vr_seed=  now.tv_usec + my_pid;
  vr_rand_setSeed(&vr_rand, vr_seed);

  ROUNDINGMODE=VR_NATIVE; //Default value

  char* vrm=std::getenv("VERROU_LIBM_ROUNDING_MODE");
  if(vrm==NULL){
    vrm=std::getenv("VERROU_ROUNDING_MODE");
  }

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
    if(envString==std::string("native")){
      ROUNDINGMODE=VR_NATIVE;
    }
  }

  initLibMathCounter();
}


void __attribute__((destructor)) finalyze_interlibmath(){
  printCounter();
};


