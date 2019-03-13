
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

  const std::string& name()const{
    return name_;
  }

private:
  void load_real_sym(void**fctPtr, std::string name ){
    (*fctPtr) =dlsym(RTLD_NEXT, name.c_str());
    if(*fctPtr==NULL){
      std::cerr << "Problem with function "<< name<<std::endl;
    }
  }

  //Attributs
  float (*real_name_float)(float) ;
  double (*real_name_double)(double) ;
  long double (*real_name_long_double)(long double) ;
  std::string name_;
};

class myLibMathFunction2{
public:
  myLibMathFunction2(std::string name):name_(name){
    load_real_sym((void**)&(real_name_float) , name +std::string("f"));
    load_real_sym((void**)&(real_name_double) , name);
    load_real_sym((void**)&(real_name_long_double) , name +std::string("l"));
  }

  inline double apply(double a, double b)const{
    return real_name_double(a,b);
  }

  inline long double apply(long double a, long double b)const{
    return real_name_long_double(a,b);
  }

  inline float apply(float a, float b)const{
    return real_name_float(a,b);
  }

  const std::string& name()const{
    return name_;
  }

private:
  void load_real_sym(void**fctPtr, std::string name ){
    (*fctPtr) =dlsym(RTLD_NEXT, name.c_str());
    if(*fctPtr==NULL){
      std::cerr << "Problem with function "<< name<<std::endl;
    }
  }

  //Attributs
  float (*real_name_float)(float,float) ;
  double (*real_name_double)(double,double) ;
  long double (*real_name_long_double)(long double, long double) ;
  std::string name_;
};



enum Function1Name {enumcos, enumsin, enumerf, enumsqrt,
		    enumlog, enumexp, enumtan, enumasin,
		    enumacos, enumatan,
		    enum_libm_function1_name_size};

myLibMathFunction1 function1NameTab[enum_libm_function1_name_size]={myLibMathFunction1("cos"),
								    myLibMathFunction1("sin"),
								    myLibMathFunction1("erf"),
								    myLibMathFunction1("sqrt"),
								    myLibMathFunction1("log"),
								    myLibMathFunction1("exp"),
								    myLibMathFunction1("tan"),
								    myLibMathFunction1("asin"),
								    myLibMathFunction1("acos"),
								    myLibMathFunction1("atan")};
enum Function2Name {enumatan2,
		    enum_libm_function2_name_size};

myLibMathFunction2 function2NameTab[enum_libm_function2_name_size]={myLibMathFunction2("atan2")};



unsigned int libMathCounter1[enum_libm_function1_name_size][3][2];
unsigned int libMathCounter2[enum_libm_function2_name_size][3][2];

void initLibMathCounter(){
  for(int i=0; i< enum_libm_function1_name_size;i++){
    for(int j=0; j< 3; j++){
      libMathCounter1[i][j][0]=0;
      libMathCounter1[i][j][1]=0;
    }
  }
  for(int i=0; i< enum_libm_function2_name_size;i++){
    for(int j=0; j< 3; j++){
      libMathCounter2[i][j][0]=0;
      libMathCounter2[i][j][1]=0;
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
inline void incCounter1(){
  libMathCounter1[ENUM_LIBM][realTypeIndex<REALTYPE>::index][INST]++;
}

template<class REALTYPE, int ENUM_LIBM, int INST>
inline void incCounter2(){
  libMathCounter2[ENUM_LIBM][realTypeIndex<REALTYPE>::index][INST]++;
}



unsigned int getCounter(int nbParam, int index,  int type, int isInst){
  if(nbParam==1){
    return libMathCounter1[index][type][isInst];
  }
  if(nbParam==2){
    return libMathCounter2[index][type][isInst];
  }
  return 0;
};

void printCounter(){
  std::cerr << "=="<<my_pid<<"== " << "Interlibm counter " <<std::endl;
  std::cerr << "=="<<my_pid<<"== " << "\t\t Total \tInstrumented" <<std::endl;

  for(int nbParam=1; nbParam <=2; nbParam++){
    int paramSize= (int)enum_libm_function1_name_size;
    if(nbParam==2){
      paramSize=enum_libm_function1_name_size;
    }

    for(int i=0; i< paramSize;i++){
      std::cerr << "=="<<my_pid<<"== ";
      std::cerr<<  "---------------------------------------------------"<<std::endl;
      std::cerr << "=="<<my_pid<<"== ";
      if(nbParam==1){
	std::cerr<< function1NameTab[i].name();
      }
      if(nbParam==2){
	std::cerr<< function2NameTab[i].name();
      }

      int total=0;
      int totalInst=0;
      for(int j=0;j<3;j++){
	total+=getCounter(nbParam,i,j,0)+getCounter(nbParam,i,j,1);
	totalInst+=getCounter(nbParam,i,j,0);
      }

      std::cerr<< "\t\t" <<  total << "\t" << totalInst<<std::endl;
      if(total!=0){
	std::cerr << "=="<<my_pid<<"== ";
	std::cerr<< " `-" " flt ";
	std::cerr<< "\t" <<  getCounter(nbParam,i,0,0)+getCounter(nbParam,i,0,1)  << "\t" << getCounter(nbParam,i,0,0)<<std::endl;

	std::cerr << "=="<<my_pid<<"== ";
	std::cerr<< " `-" " dbl ";
	std::cerr<< "\t" <<  getCounter(nbParam,i,1,0)+getCounter(nbParam,i,1,1)  << "\t" << getCounter(nbParam,i,1,0)<<std::endl;

	std::cerr << "=="<<my_pid<<"== ";
	std::cerr<< " `-" " lgd ";
	std::cerr<< "\t" <<  getCounter(nbParam,i,2,0)+getCounter(nbParam,i,2,1)  << "\t" << getCounter(nbParam,i,2,0)<<std::endl;
      }
    }
  }
}


template<class LIBMQ, typename REALTYPE >
class libMathFunction1{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;

#ifdef DEBUG_PRINT_OP
  static const char* OpName(){return "libmath ?";}
#endif

  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    __float128 ref=LIBMQ::apply((__float128)a);
    return (RealType)ref;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {\
    const RealType & a(p.arg1);
    __float128 ref=LIBMQ::apply((__float128)a);
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

template<class LIBMQ, typename REALTYPE >
class libMathFunction2{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,2> PackArgs;

#ifdef DEBUG_PRINT_OP
  static const char* OpName(){return "libmath ?";}
#endif

  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);

    __float128 ref=LIBMQ::apply((__float128)a, (__float128)b);
    return (RealType)ref;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {\
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);

    __float128 ref=LIBMQ::apply((__float128)a,(__float128)b);
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


#define DEFINE_INTERP_LIBM1_C_IMPL(FCT)					\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a){return FCT##q(a);}		\
  };									\
  extern "C"{								\
  double FCT (double a){						\
    if(ROUNDINGMODE==VR_NATIVE){					\
      incCounter1<double, enum##FCT ,1>();				\
      return function1NameTab[enum##FCT].apply(a);			\
    }else{								\
      incCounter1<double, enum##FCT ,0>();				\
      typedef OpWithSelectedRoundingMode<libMathFunction1<libmq##FCT,double>,double > Op; \
      double res;							\
      Op::apply(Op::PackArgs(a) ,&res,NULL);				\
      return res;							\
    }									\
  }									\
									\
  float FCT##f (float a){						\
    if(ROUNDINGMODE==VR_NATIVE){					\
      incCounter1<float, enum##FCT ,1>();				\
      return function1NameTab[enum##FCT].apply(a);			\
    }else{								\
      incCounter1<float, enum##FCT,0>();					\
      typedef OpWithSelectedRoundingMode<libMathFunction1<libmq##FCT,float>,float > Op; \
      float res;							\
      Op::apply(Op::PackArgs(a) ,&res,NULL);				\
      return res;							\
    }									\
  }									\
									\
  long double FCT##l (long double a){					\
    incCounter1<long double, enum##FCT,1>();				\
    return function1NameTab[enum##FCT].apply(a);			\
  }									\
};

#define DEFINE_INTERP_LIBM2_C_IMPL(FCT)					\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a,__float128 b){return FCT##q(a,b);} \
  };									\
  extern "C"{								\
    double FCT (double a, double b){					\
    if(ROUNDINGMODE==VR_NATIVE){					\
      incCounter2<double, enum##FCT ,1>();				\
      return function2NameTab[enum##FCT].apply(a,b);			\
    }else{								\
      incCounter2<double, enum##FCT ,0>();				\
      typedef OpWithSelectedRoundingMode<libMathFunction2<libmq##FCT,double>,double > Op; \
      double res;							\
      Op::apply(Op::PackArgs(a,b) ,&res,NULL);				\
      return res;							\
    }									\
  }									\
									\
    float FCT##f (float a, float b){						\
    if(ROUNDINGMODE==VR_NATIVE){					\
      incCounter2<float, enum##FCT ,1>();				\
      return function2NameTab[enum##FCT].apply(a,b);			\
    }else{								\
      incCounter2<float, enum##FCT,0>();					\
      typedef OpWithSelectedRoundingMode<libMathFunction2<libmq##FCT,float>,float > Op; \
      float res;							\
      Op::apply(Op::PackArgs(a,b) ,&res,NULL);				\
      return res;							\
    }									\
  }									\
									\
    long double FCT##l (long double a, long double b){				\
    incCounter2<long double, enum##FCT,1>();				\
    return function2NameTab[enum##FCT].apply(a,b);			\
  }									\
};

  DEFINE_INTERP_LIBM1_C_IMPL(cos);
  DEFINE_INTERP_LIBM1_C_IMPL(sin);
  DEFINE_INTERP_LIBM1_C_IMPL(erf);
  DEFINE_INTERP_LIBM1_C_IMPL(sqrt);
  DEFINE_INTERP_LIBM1_C_IMPL(log);
  DEFINE_INTERP_LIBM1_C_IMPL(exp);
  DEFINE_INTERP_LIBM1_C_IMPL(tan);
  DEFINE_INTERP_LIBM1_C_IMPL(asin);
  DEFINE_INTERP_LIBM1_C_IMPL(acos);
  DEFINE_INTERP_LIBM1_C_IMPL(atan);

  DEFINE_INTERP_LIBM2_C_IMPL(atan2);

#undef DEFINE_INTERP_LIBM1_C_IMPL
#undef DEFINE_INTERP_LIBM2_C_IMPL



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
