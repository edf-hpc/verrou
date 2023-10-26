
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cstdlib>

#include <float.h>
#include <quadmath.h>
#include "../interflop_backends/interflop_verrou/interflop_verrou.h"
#include "../interflop_backends/interflop_verrou/vr_rand.h"
#include "../interflop_backends/interflop_verrou/vr_roundingOp.hxx"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>


unsigned int my_pid;

#ifdef INTERLIBM_STAND_ALONE
char libraryName[]="verrou-interLibm";
#else
char libraryName[]="*vgpreload_verrou-*.so";
#endif

char fileName[]=__FILE__;





#ifdef INTERLIBM_STAND_ALONE
#include "stacktrace.cxx"

void printNan(){
  std::cerr << "\n=="<<my_pid<<"== NaN Detected:"<< std::endl;
  std::cerr << Backtrace(3);
}
void printInf(){
  std::cerr << "\n=="<<my_pid<<"== +/- Inf Detected:"<< std::endl;
  std::cerr << Backtrace(3);
}

void (*vr_nanHandler)()=printNan;
void (*vr_infHandler)()=printInf;

#define VERROU_IS_INSTRUMENTED_FLOAT true
#define VERROU_IS_INSTRUMENTED_DOUBLE true
#define VERROU_IS_INSTRUMENTED_LDOUBLE true
#define VERROU_GET_LIBM_ROUNDING VR_NATIVE
#define VERROU_COUNT_OP true

#else

#define Bool bool
#include "verrou/verrou.h"

void signalNan(){
  VERROU_NAN_DETECTED;
}
void signalInf(){
  VERROU_INF_DETECTED;
}


void (*vr_nanHandler)()=signalNan;
void (*vr_infHandler)()=signalInf;
#endif



vr_RoundingMode ROUNDINGMODE;
void (*vr_cancellationHandler)(int)=NULL;
void (*vr_panicHandler)(const char*)=NULL;


class myLibMathFunction1{
public:
  myLibMathFunction1(std::string name, uint64_t enumName, uint64_t line):name_(name),
									 hash_(enumName+nbOpHash),
									 line_(line)
  {
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

  uint64_t getHash()const{
    return hash_;
  }

  uint64_t getLine()const{
    return line_;
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
  uint64_t hash_;
  uint64_t line_;
};

class myLibMathFunction2{
public:
  myLibMathFunction2(std::string name, uint64_t enumName, uint64_t line):name_(name),
									 hash_(enumName+ nbOpHash),
									 line_(line)
  {
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
  uint64_t getHash()const{
    return hash_;
  }

  uint64_t getLine()const{
    return line_;
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
  uint64_t hash_;
  uint64_t line_;
};



//shell LIST1="acos acosh asin asinh atan atanh cbrt erf exp exp2 expm1 log log10 log1p log2 tgamma lgamma sin sinh cos cosh sqrt tan tanh j0 j1 y0 y1"
enum Function1Name: uint64_t {
  //shell comand to generate:  for i in $LIST1 ; do  echo "enum$i,"; done;
  enumacos,
  enumacosh,
  enumasin,
  enumasinh,
  enumatan,
  enumatanh,
  enumcbrt,
  enumerf,
  enumexp,
  enumexp2,
  enumexpm1,
  enumlog,
  enumlog10,
  enumlog1p,
  enumlog2,
  enumtgamma,
  enumlgamma,
  enumsin,
  enumsinh,
  enumcos,
  enumcosh,
  enumsqrt,
  enumtan,
  enumtanh,
  enumj0,
  enumj1,
  enumy0,
  enumy1,
  //fin shell
  enum_libm_function1_name_size};

myLibMathFunction1 function1NameTab[enum_libm_function1_name_size]={
//shell command to generate  for i in $LIST1 ; do  echo "myLibMathFunction1(\"$i\", enum$i,__LINE__),"; done;
  myLibMathFunction1("acos", enumacos, __LINE__),
  myLibMathFunction1("acosh", enumacosh, __LINE__),
  myLibMathFunction1("asin", enumasin, __LINE__),
  myLibMathFunction1("asinh", enumasinh, __LINE__),
  myLibMathFunction1("atan", enumatan, __LINE__),
  myLibMathFunction1("atanh", enumatanh, __LINE__),
  myLibMathFunction1("cbrt", enumcbrt, __LINE__),
  myLibMathFunction1("erf", enumerf, __LINE__),
  myLibMathFunction1("exp", enumexp, __LINE__),
  myLibMathFunction1("exp2", enumexp2, __LINE__),
  myLibMathFunction1("expm1", enumexpm1, __LINE__),
  myLibMathFunction1("log", enumlog, __LINE__),
  myLibMathFunction1("log10", enumlog10, __LINE__),
  myLibMathFunction1("log1p", enumlog1p, __LINE__),
  myLibMathFunction1("log2", enumlog2, __LINE__),
  myLibMathFunction1("tgamma", enumtgamma, __LINE__),
  myLibMathFunction1("lgamma", enumlgamma, __LINE__),
  myLibMathFunction1("sin", enumsin, __LINE__),
  myLibMathFunction1("sinh", enumsinh, __LINE__),
  myLibMathFunction1("cos", enumcos, __LINE__),
  myLibMathFunction1("cosh", enumcosh, __LINE__),
  myLibMathFunction1("sqrt", enumsqrt, __LINE__),
  myLibMathFunction1("tan", enumtan, __LINE__),
  myLibMathFunction1("tanh", enumtanh, __LINE__),
  myLibMathFunction1("j0", enumj0, __LINE__),
  myLibMathFunction1("j1", enumj1, __LINE__),
  myLibMathFunction1("y0", enumy0, __LINE__),
  myLibMathFunction1("y1", enumy1, __LINE__),
};
enum Function2Name : uint64_t{
  enumatan2,
  enumfmod,
  enumhypot,
  enumpow,
  enumfdim,
  enumremainder,
  enum_libm_function2_name_size};

myLibMathFunction2 function2NameTab[enum_libm_function2_name_size]={
  myLibMathFunction2("atan2",enumatan2, __LINE__),
  myLibMathFunction2("fmod", enumfmod,  __LINE__),
  myLibMathFunction2("hypot", enumhypot, __LINE__),
  myLibMathFunction2("pow", enumpow, __LINE__),
  myLibMathFunction2("fdim",enumfdim, __LINE__),
  myLibMathFunction2("remainder", enumremainder, __LINE__),
};



unsigned int libMathCounter1[enum_libm_function1_name_size][3][2];
unsigned int libMathCounter2[enum_libm_function2_name_size][3][2];

unsigned int* cacheInstrumentStatus1;
unsigned int* cacheInstrumentStatus2;



void initLibMathCounter(){
  for(int i=0; i< (int)enum_libm_function1_name_size;i++){
    for(int j=0; j< 3; j++){
      libMathCounter1[i][j][0]=0;
      libMathCounter1[i][j][1]=0;
    }
  }
  for(int i=0; i< (int)enum_libm_function2_name_size;i++){
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

const char*  verrou_rounding_mode_name_redefined (enum vr_RoundingMode mode) {
  switch (mode) {
  case VR_NEAREST:
    return "NEAREST";
  case VR_UPWARD:
    return "UPWARD";
  case VR_DOWNWARD:
    return "DOWNWARD";
  case VR_ZERO:
    return "TOWARD_ZERO";
  case VR_AWAY_ZERO:
    return "AWAY_ZERO";
  case VR_RANDOM:
    return "RANDOM";
  case VR_RANDOM_DET:
    return "RANDOM_DET";
  case VR_RANDOM_COMDET:
    return "RANDOM_COMDET";
  case VR_RANDOM_SCOMDET:
    return "RANDOM_SCOMDET";
  case VR_SR_MONOTONIC:
    return "SR_MONOTONIC";
  case VR_SR_SMONOTONIC:
    return "SR_SMONOTONIC";
  case VR_AVERAGE:
    return "AVERAGE";
  case VR_AVERAGE_DET:
    return "AVERAGE_DET";
  case VR_AVERAGE_COMDET:
    return "AVERAGE_COMDET";
  case VR_AVERAGE_SCOMDET:
    return "AVERAGE_SCOMDET";
  case VR_PRANDOM:
    return "PRANDOM";
  case VR_PRANDOM_DET:
    return "PRANDOM_DET";
  case VR_PRANDOM_COMDET:
    return "PRANDOM_COMDET";
  case VR_FARTHEST:
    return "FARTHEST";
  case VR_FLOAT:
    return "FLOAT";
  case VR_NATIVE:
    return "NATIVE";
  case VR_FTZ:
    std::cerr<< "Rounding VR_FTZ not yet implemented in interlibmath"<<std::endl;
    exit(1);
    return "FTZ";


  }

  return "undefined";
}



#ifndef INTERLIBM_STAND_ALONE
void generateExcludeSource(){
  for(int nbParam=1; nbParam <=2; nbParam++){
    int paramSize= (int)enum_libm_function1_name_size;
    if(nbParam==2){
      paramSize=(int)enum_libm_function2_name_size;
    }

    for(int i=0; i< paramSize;i++){
      std::string functionName;
      int line;
      if(nbParam==1){
	functionName=function1NameTab[i].name();
	line=function1NameTab[i].getLine();
      }
      if(nbParam==2){
	functionName=function2NameTab[i].name();
	line=function1NameTab[i].getLine();
      }
      if( getCounter(nbParam,i,0,0)!=0){  //float
	std::string fctName=functionName+std::string("f");
	VERROU_GENERATE_EXCLUDE_SOURCE(fctName.c_str(), &line, fileName, libraryName);
      }
      if( getCounter(nbParam,i,1,0)!=0){ //double
	VERROU_GENERATE_EXCLUDE_SOURCE(functionName.c_str(), &line, fileName, libraryName);
      }
      if( getCounter(nbParam,i,2,0)!=0){//ldouble
	std::string fctName=functionName+std::string("l");
	VERROU_GENERATE_EXCLUDE_SOURCE(fctName.c_str(), &line, fileName,libraryName);
      }
    }
  }
}
#endif

void printCounter(){
  std::cerr << "=="<<my_pid<<"== " << "Interlibm counter ( ROUNDINGMODE="<< verrou_rounding_mode_name_redefined (ROUNDINGMODE)<<" )"<<std::endl;
  std::cerr << "=="<<my_pid<<"== " << "\t\t Total \tInstrumented" <<std::endl;

  for(int nbParam=1; nbParam <=2; nbParam++){
    int paramSize= (int)enum_libm_function1_name_size;
    if(nbParam==2){
      paramSize=(int)enum_libm_function2_name_size;
    }

    for(int i=0; i< paramSize;i++){
      int total=0;
      int totalInst=0;
      for(int j=0;j<3;j++){
	total+=getCounter(nbParam,i,j,0)+getCounter(nbParam,i,j,1);
	totalInst+=getCounter(nbParam,i,j,0);
      }

      if(total!=0){
	std::cerr << "=="<<my_pid<<"== ";
	std::cerr<<  "---------------------------------------------------"<<std::endl;
	std::cerr << "=="<<my_pid<<"== ";

	if(nbParam==1){
	  std::cerr<< function1NameTab[i].name();
	}
	if(nbParam==2){
	  std::cerr<< function2NameTab[i].name();
	}

	std::cerr<< "\t\t" <<  total << "\t" << totalInst<<std::endl;

	int totalFloat=getCounter(nbParam,i,0,0)+getCounter(nbParam,i,0,1);
	if(totalFloat>0){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " flt ";
	  std::cerr<< "\t" <<  totalFloat  << "\t" << getCounter(nbParam,i,0,0)<<std::endl;
	}

	int totalDouble=getCounter(nbParam,i,1,0)+getCounter(nbParam,i,1,1);
	if(totalDouble>0){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " dbl ";
	  std::cerr<< "\t" <<  totalDouble  << "\t" << getCounter(nbParam,i,1,0)<<std::endl;
	}

	int totalLDouble= getCounter(nbParam,i,2,0)+getCounter(nbParam,i,2,1);
	if(totalLDouble){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " lgd ";
	  std::cerr<< "\t" << totalLDouble  << "\t" << getCounter(nbParam,i,2,0)<<std::endl;
	}
      }
    }
  }
}


template<class LIBMQ, typename REALTYPE >
class libMathFunction1{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "libmath ?";}
  static inline uint32_t getHash(){return LIBMQ::getHash();}

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

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    return r.hash(p, hashOp);
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    return r.hash(p, hashOp);
  };

  static inline void check(const PackArgs& p, const RealType& d){
  };

};

template<class LIBMQ, typename REALTYPE >
class libMathFunction2{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,2> PackArgs;
  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "libmath ?";}
  static inline uint32_t getHash(){return LIBMQ::getHash();}

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

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const RealType pmin(std::min<RealType>(p.arg1,p.arg2));
    const RealType pmax(std::max<RealType>(p.arg1,p.arg2));
    const vr_packArg<RealType,2> pcom(pmin,pmax);
    const uint32_t hashOp(getHash());
    return r.hash(pcom,hashOp);
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    return r.hash(p, hashOp);
  };

};


// template<class REALTYPE>
// REALTYPE MAGIC(constraint_m1p1)(const REALTYPE& x ){
//   if(x>1) return 1.;
//   if(x<-1) return -1.;
//   return x;
// }

#ifdef INTERLIBM_STAND_ALONE
bool isInstrumented1(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
bool isInstrumented2(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
#else
bool isInstrumented1(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  unsigned int index= functionType * enum_libm_function1_name_size+ functionEnum;
  int line=( function2NameTab[functionEnum]).getLine();
  if(  cacheInstrumentStatus1[index]==0){
    if(VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(functionName, &line, fileName, libraryName)){
      cacheInstrumentStatus1[index]=1;
    }else{
      cacheInstrumentStatus1[index]=2;
    }
  }
  if( cacheInstrumentStatus1[index]==1){
    return true;
  }else{
    return false;
  }
}

bool isInstrumented2(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  unsigned int index= functionType * enum_libm_function2_name_size+ functionEnum;
  int line=( function2NameTab[functionEnum]).getLine();
  if(  cacheInstrumentStatus2[index]==0){
    if(VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(functionName, &line, fileName, libraryName)){
      cacheInstrumentStatus2[index]=1;
    }else{
      cacheInstrumentStatus2[index]=2;
    }
  }
  if( cacheInstrumentStatus2[index]==1){
    return true;
  }else{
    return false;
  }
}
#endif


#define STRINGIFY(A) #A

#define DEFINE_INTERP_LIBM1_C_IMPL(FCT)					\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a){return FCT##q(a);}		\
    static __uint64_t getHash(){return enum##FCT; }                     \
};									\
  extern "C"{								\
  double FCT (double a){						\
    const char fctStr[]=STRINGIFY(FCT);					\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented1(fctStr, enum##FCT,1); \
    if(ROUNDINGMODE==VR_NATIVE || !(isInstrumented)){			\
    if(isInstrumented){							\
      incCounter1<double, enum##FCT ,0>();				\
    }else{								\
      incCounter1<double, enum##FCT ,1>();				\
    }									\
    return function1NameTab[enum##FCT].apply(a);			\
    }else{								\
      incCounter1<double, enum##FCT ,0>();				\
      typedef OpWithDynSelectedRoundingMode<libMathFunction1<libmq##FCT,double> > Op; \
      double res;							\
      Op::apply(Op::PackArgs(a) ,&res,NULL);				\
      return res;							\
    }									\
  }									\
									\
  float FCT##f (float a){						\
  char fctStr[30]=STRINGIFY(FCT##f);			\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented1(fctStr, enum##FCT,0);\
    if(ROUNDINGMODE==VR_NATIVE || !(isInstrumented)){			\
      if(isInstrumented){						\
	incCounter1<float, enum##FCT ,0>();				\
      }else{								\
	incCounter1<float, enum##FCT ,1>();				\
      }									\
      return function1NameTab[enum##FCT].apply(a);			\
    }else{								\
      incCounter1<float, enum##FCT,0>();				\
      typedef OpWithDynSelectedRoundingMode<libMathFunction1<libmq##FCT,float> > Op; \
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

#define DEFINE_INTERP_LIBM2_C_IMPL(FCT)				\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a,__float128 b){return FCT##q(a,b);} \
    static __uint64_t getHash(){return enum##FCT; }			\
};									\
  extern "C"{								\
    double FCT (double a, double b){					\
      const char fctStr[]=#FCT;						\
      const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented2(fctStr, enum##FCT,1); \
      if(ROUNDINGMODE==VR_NATIVE ||(!isInstrumented)){			\
	if(isInstrumented){						\
	  incCounter2<double, enum##FCT ,0>();				\
	}else{								\
	  incCounter2<double, enum##FCT ,1>();				\
	}								\
	return function2NameTab[enum##FCT].apply(a,b);			\
      }else{								\
	incCounter2<double, enum##FCT ,0>();				\
	typedef OpWithDynSelectedRoundingMode<libMathFunction2<libmq##FCT,double> > Op; \
	double res;							\
	Op::apply(Op::PackArgs(a,b) ,&res,NULL);			\
	return res;							\
      }									\
    }									\
									\
    float FCT##f (float a, float b){					\
    const char fctStr[]=STRINGIFY(FCT##f);				\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented2(fctStr, enum##FCT,0);\
      if(ROUNDINGMODE==VR_NATIVE ||(!isInstrumented)){			\
	if(isInstrumented){						\
	  incCounter2<float, enum##FCT ,0>();				\
	}else{								\
	  incCounter2<float, enum##FCT ,1>();				\
	}								\
	return function2NameTab[enum##FCT].apply(a,b);			\
      }else{								\
	incCounter2<float, enum##FCT,0>();				\
	typedef OpWithDynSelectedRoundingMode<libMathFunction2<libmq##FCT,float> > Op; \
	float res;							\
	Op::apply(Op::PackArgs(a,b) ,&res,NULL);			\
	return res;							\
      }									\
}									\
									\
  long double FCT##l (long double a, long double b){			\
    incCounter2<long double, enum##FCT,1>();				\
    return function2NameTab[enum##FCT].apply(a,b);			\
  }									\
};

//shell for i in $LIST1 ; do  echo " DEFINE_INTERP_LIBM1_C_IMPL($i);"; done;
 DEFINE_INTERP_LIBM1_C_IMPL(acos);
 DEFINE_INTERP_LIBM1_C_IMPL(acosh);
 DEFINE_INTERP_LIBM1_C_IMPL(asin);
 DEFINE_INTERP_LIBM1_C_IMPL(asinh);
 DEFINE_INTERP_LIBM1_C_IMPL(atan);
 DEFINE_INTERP_LIBM1_C_IMPL(atanh);
 DEFINE_INTERP_LIBM1_C_IMPL(cbrt);
 DEFINE_INTERP_LIBM1_C_IMPL(erf);
 DEFINE_INTERP_LIBM1_C_IMPL(exp);
// DEFINE_INTERP_LIBM1_C_IMPL(exp2);
 DEFINE_INTERP_LIBM1_C_IMPL(expm1);
 DEFINE_INTERP_LIBM1_C_IMPL(log);
 DEFINE_INTERP_LIBM1_C_IMPL(log10);
 DEFINE_INTERP_LIBM1_C_IMPL(log1p);
 DEFINE_INTERP_LIBM1_C_IMPL(log2);
 DEFINE_INTERP_LIBM1_C_IMPL(tgamma);
 DEFINE_INTERP_LIBM1_C_IMPL(lgamma);
 DEFINE_INTERP_LIBM1_C_IMPL(sin);
 DEFINE_INTERP_LIBM1_C_IMPL(sinh);
 DEFINE_INTERP_LIBM1_C_IMPL(cos);
 DEFINE_INTERP_LIBM1_C_IMPL(cosh);
 DEFINE_INTERP_LIBM1_C_IMPL(sqrt);
 DEFINE_INTERP_LIBM1_C_IMPL(tan);
 DEFINE_INTERP_LIBM1_C_IMPL(tanh);
 DEFINE_INTERP_LIBM1_C_IMPL(j0);
 DEFINE_INTERP_LIBM1_C_IMPL(j1);
 DEFINE_INTERP_LIBM1_C_IMPL(y0);
 DEFINE_INTERP_LIBM1_C_IMPL(y1);


DEFINE_INTERP_LIBM2_C_IMPL(atan2);
DEFINE_INTERP_LIBM2_C_IMPL(fmod);
DEFINE_INTERP_LIBM2_C_IMPL(hypot);
DEFINE_INTERP_LIBM2_C_IMPL(pow);
DEFINE_INTERP_LIBM2_C_IMPL(fdim);
DEFINE_INTERP_LIBM2_C_IMPL(remainder);

#undef DEFINE_INTERP_LIBM1_C_IMPL
#undef DEFINE_INTERP_LIBM2_C_IMPL



void __attribute__((constructor)) init_interlibmath(){
  struct timeval now;
  gettimeofday(&now, NULL);
  my_pid = getpid();
  uint64_t vr_seed=  now.tv_usec + my_pid;
  vr_rand_setSeed(&vr_rand, vr_seed);

  ROUNDINGMODE = VERROU_GET_LIBM_ROUNDING; // VR_NATIVE; //Default value

#ifdef INTERLIBM_STAND_ALONE
  char* vrm=std::getenv("VERROU_LIBM_ROUNDING_MODE");
  if(vrm==NULL){
    vrm=std::getenv("VERROU_ROUNDING_MODE");
  }

  if(vrm!=NULL){
    std::string envString(vrm);
    if(envString==std::string("random")){
      ROUNDINGMODE=VR_RANDOM;
    }
    if(envString==std::string("random_det")){
      ROUNDINGMODE=VR_RANDOM_DET;
    }
    if(envString==std::string("random_comdet")){
      ROUNDINGMODE=VR_RANDOM_COMDET;
    }
    if(envString==std::string("random_scomdet")){
      std::cerr<< "Rounding RANDOM_SCOMDET not yet implemented in interlibmath"<<std::endl;
      exit(1);
      ROUNDINGMODE=VR_RANDOM_SCOMDET;
    }
    if(envString==std::string("sr_monotonic")){
      ROUNDINGMODE=VR_SR_MONOTONIC;
    }
    if(envString==std::string("sr_smonotonic")){
      ROUNDINGMODE=VR_SR_SMONOTONIC;
    }
    if(envString==std::string("average")){
      ROUNDINGMODE=VR_AVERAGE;
    }
    if(envString==std::string("average_det")){
      ROUNDINGMODE=VR_AVERAGE_DET;
    }
    if(envString==std::string("average_comdet")){
      ROUNDINGMODE=VR_AVERAGE_COMDET;
    }
    if(envString==std::string("average_scomdet")){
      std::cerr<< "Rounding RANDOM_SCOMDET not yet implemented in interlibmath"<<std::endl;
      exit(1);
      ROUNDINGMODE=VR_AVERAGE_SCOMDET;
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
    if(envString==std::string("away_zero")){
      ROUNDINGMODE=VR_AWAY_ZERO;
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
    if(envString==std::string("ftz")){
      std::cerr<< "Rounding VR_FTZ not yet implemented in interlibmath"<<std::endl;
      exit(1);
    }
  }
#else
  cacheInstrumentStatus1=(unsigned int*)calloc(3 * enum_libm_function1_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus1, 3 * enum_libm_function1_name_size * sizeof(unsigned int));

  cacheInstrumentStatus2=(unsigned int*)calloc(3 * enum_libm_function2_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus2, 3 * enum_libm_function2_name_size * sizeof(unsigned int));
#endif
  initLibMathCounter();
}


void __attribute__((destructor)) finalyze_interlibmath(){
#ifndef INTERLIBM_STAND_ALONE
  generateExcludeSource();
#endif
  if(VERROU_COUNT_OP){
    printCounter();
  }
#ifndef INTERLIBM_STAND_ALONE
  free(cacheInstrumentStatus1);
  free(cacheInstrumentStatus2);
#endif
};
