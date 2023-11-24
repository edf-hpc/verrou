
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cstdlib>

#include <float.h>
#include <quadmath.h>

#define LIBM_DEBUG
#define WRITE_DEBUG(STR) write(0, STR, sizeof(STR));


#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>

#include <complex.h>

#include "../interflop_backends/interflop_verrou/interflop_verrou.h"
#include "../interflop_backends/interflop_verrou/vr_rand.h"
#include "../interflop_backends/interflop_verrou/vr_roundingOp.hxx"

unsigned int my_pid;



#ifdef INTERLIBM_STAND_ALONE
char libraryName[]="verrou-interLibm";
#else
char libraryName[]="builtin(vgpreload_verrou-ARCH.so)";
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
#define VERROU_GET_LIBM_ROUNDING_NO_INST VR_NATIVE
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
vr_RoundingMode ROUNDINGMODE_NOINST;

void (*vr_cancellationHandler)(int)=NULL;
void (*vr_panicHandler)(const char*)=NULL;


typedef enum libMparity: size_t {
			   enumNoParity,
			   enumEven,
			   enumOdd
} libMparity_t;


typedef enum libM2parity: size_t {
				    enum2NoParity,
				    enum2Even,
				    enum2OddFirst
} libM2parity_t;


void load_postfix_sym(void**fctPtr, const char* name, const char* postFix){
  char localName[20];
  size_t i=0 ;
  while(name[i]!=0){
    localName[i]=name[i];
    i++;
  }
  size_t j=0;
  while(postFix[j]!=0){
    localName[i]=postFix[j];
    i++, j++;
  }
  localName[i]=0;

  (*fctPtr) =dlsym(RTLD_NEXT, localName);
  if(*fctPtr==NULL){
    //write : to avoid libc
    write(1, "Problem with function ", sizeof("Problem with function "));
    write(1, localName, i);
    write(1, "\n",1);
  }
}

// strncat redefined to avoid libc call during initialization
void strncat_redefined(char* nameOut, const char* nameIn, size_t n){
  size_t i=0;
  while( nameIn[i]!=0 && i<n){
    nameOut[i]=nameIn[i];
    i++;
  }
  nameOut[i]=0;
}
#define NAME_FUNCTION_SIZE 25



template<class DERIVED>
class myLibMathGen{
public:
  myLibMathGen(const char* name, uint64_t enumName, uint64_t line): hash_(enumName+nbOpHash),
								    line_(line)

  {
    load_postfix_sym((void**)&(static_cast<DERIVED*>(this)->real_name_float),       name, "f");
    load_postfix_sym((void**)&(static_cast<DERIVED*>(this)->real_name_double),      name, "" );
    load_postfix_sym((void**)&(static_cast<DERIVED*>(this)->real_name_long_double), name, "l");
    strncat_redefined(name_,name, NAME_FUNCTION_SIZE);
  }

  myLibMathGen(const myLibMathGen& rhs):hash_(rhs.hash_),
					line_(rhs.line_){
    strncat_redefined(name_,rhs.name_, NAME_FUNCTION_SIZE);
  }

  virtual ~myLibMathGen(){}

  void liberate(){
    dlclose(&(static_cast<DERIVED*>(this)->real_name_float));
    dlclose(&(static_cast<DERIVED*>(this)->real_name_double));
    dlclose(&(static_cast<DERIVED*>(this)->real_name_long_double));
  }

  inline const char* name()const{return name_;}
  inline uint64_t getHash()const{return hash_;}
  inline uint64_t getLine()const{return line_;}

private:
  uint64_t hash_;
  uint64_t line_;
  char name_[NAME_FUNCTION_SIZE];

};

class myLibMathFunction1:public myLibMathGen<myLibMathFunction1>{
public:
  myLibMathFunction1(const char* name, uint64_t enumName, uint64_t line):
    myLibMathGen<myLibMathFunction1>(name,enumName,line) {
  }

  myLibMathFunction1(const myLibMathFunction1& rhs):
    myLibMathGen<myLibMathFunction1>(rhs),
    real_name_float(rhs.real_name_float),
    real_name_double(rhs.real_name_double),
    real_name_long_double(rhs.real_name_long_double){
  }

  ~myLibMathFunction1(){}
  void liberate(){}//Comment debuggy liberate

  template<class REALTYPE>
  inline REALTYPE apply(const vr_packArg<REALTYPE,1>& p)const{
    return apply(p.arg1);
  }
  inline double apply(double a)const{ return real_name_double(a);}
  inline long double apply(long double a)const{ return real_name_long_double(a);}
  inline float apply(float a)const{ return real_name_float(a); }

  inline void setParity(libMparity_t p){parity_=p;}
  inline libMparity_t getParity(){return parity_;}

  //Attributs
  float (*real_name_float)(float) ;
  double (*real_name_double)(double) ;
  long double (*real_name_long_double)(long double) ;
private:
  libMparity_t parity_;
};

class myLibMathFunction1Complex: public myLibMathGen<myLibMathFunction1Complex>{
public:
  myLibMathFunction1Complex(const char* name, size_t enumName, size_t line):
    myLibMathGen<myLibMathFunction1Complex>(name, enumName,line)
  {
  }

  myLibMathFunction1Complex(const myLibMathFunction1Complex& rhs):myLibMathGen<myLibMathFunction1Complex>(rhs),
						    real_name_float(rhs.real_name_float),
						    real_name_double(rhs.real_name_double),
						    real_name_long_double(rhs.real_name_long_double){
  }

  ~myLibMathFunction1Complex(){}
  void liberate(){}//Comment debuggy liberate

  template<class REALTYPE>
  inline REALTYPE apply(const vr_packArg<REALTYPE,1>& p)const{
    return apply(p.arg1);
  }
  double _Complex apply(double _Complex a)const{ return real_name_double(a);}
  long double _Complex apply(long double _Complex a)const{return real_name_long_double(a);}
  float _Complex apply(float _Complex a)const{return real_name_float(a);}

  //Attributs
  float _Complex(*real_name_float)(float _Complex) ;
  double _Complex(*real_name_double)(double _Complex) ;
  long double (*real_name_long_double)(long double _Complex) ;
private:

};


class myLibMathFunction2: public myLibMathGen<myLibMathFunction2> {
public:
  myLibMathFunction2(const char* name, size_t enumName, uint64_t line):myLibMathGen<myLibMathFunction2>(name,enumName,line){
  }

  myLibMathFunction2(const myLibMathFunction2& rhs):myLibMathGen<myLibMathFunction2>(rhs),
						    real_name_float(rhs.real_name_float),
						    real_name_double(rhs.real_name_double),
						    real_name_long_double(rhs.real_name_long_double){
  }

  ~myLibMathFunction2(){}
  void liberate(){}//Comment debuggy liberate

  template<class REALTYPE>
  inline REALTYPE apply(const vr_packArg<REALTYPE,2>& p)const{
    return apply(p.arg1, p.arg2);
  }
  inline double apply(double a, double b)const{ return real_name_double(a,b);}
  inline long double apply(long double a, long double b)const{ return real_name_long_double(a,b);}
  inline float apply(float a, float b)const{ return real_name_float(a,b);}

  inline void setCommutativity(bool value){
    commutativity_=value;
  }
  inline bool getCommutativity()const{
    return commutativity_;
  }

  inline void setParity2(libM2parity_t p){
    parity2_=p;
  }

  inline libM2parity_t getParity2(){
    return parity2_;
  }

  //Attributs
  float (*real_name_float)(float,float) ;
  double (*real_name_double)(double,double) ;
  long double (*real_name_long_double)(long double, long double) ;
private:
  bool commutativity_;
  libM2parity_t parity2_;
};






class myLibMathFunction2IntFP: public myLibMathGen<myLibMathFunction2IntFP>{
public:
  myLibMathFunction2IntFP(const char* name, size_t enumName, size_t line): myLibMathGen(name,enumName,line){
  }

  myLibMathFunction2IntFP(const myLibMathFunction2IntFP& rhs):myLibMathGen(rhs),
							      real_name_float(rhs.real_name_float),
							      real_name_double(rhs.real_name_double),
							      real_name_long_double(rhs.real_name_long_double){
  }
  ~myLibMathFunction2IntFP(){}

  template<class REALTYPE>
  inline REALTYPE apply(const packargsIntReal<REALTYPE>& p)const{
    return apply(p.arg1, p.arg2);
  }
  inline double apply(int a, double b)const          { return real_name_double(a,b);}
  inline long double apply(int a, long double b)const{ return real_name_long_double(a,b);}
  inline float apply(int a, float b)const            { return real_name_float(a,b);}

  void liberate(){ //Implemn to debug liberate
    WRITE_DEBUG("liberate begin\n");
    dlclose(&real_name_float);
    WRITE_DEBUG("liberate entre float double\n");
    dlclose(&real_name_double);
    WRITE_DEBUG("liberate entre double long  double\n");
    dlclose(&real_name_long_double);
    WRITE_DEBUG("liberate end\n");
  }

  //attribute
  float (*real_name_float)(int,float) ;
  double (*real_name_double)(int, double) ;
  long double (*real_name_long_double)(int, long double) ;
};



class myLibMathFunction3: public myLibMathGen<myLibMathFunction3>{
public:
  myLibMathFunction3(const char* name, size_t enumName, size_t line): myLibMathGen<myLibMathFunction3>(name,enumName,line){}

  myLibMathFunction3(const myLibMathFunction3& rhs):myLibMathGen(rhs),
						    real_name_float(rhs.real_name_float),
						    real_name_double(rhs.real_name_double),
						    real_name_long_double(rhs.real_name_long_double){
  }
  ~myLibMathFunction3(){}
  void liberate(){}//Comment debuggy liberate

  template<class REALTYPE>
  inline REALTYPE apply(const vr_packArg<REALTYPE,3>& p)const{
    return apply(p.arg1, p.arg2, p.arg3);
  }
  inline double apply(double a, double b, double c)const{ return real_name_double(a,b,c);}
  inline long double apply(long double a, long double b, long double c)const{return real_name_long_double(a,b,c);}
  inline float apply(float a, float b, float c)const{return real_name_float(a,b,c);}

  //Attributs
  float (*real_name_float)(float,float,float) ;
  double (*real_name_double)(double,double,double) ;
  long double (*real_name_long_double)(long double, long double, long double) ;
};

//Remarks:
//do not need to be instrumented
//ceil trunc modf frexp fabs floor nearbyint fmod

// not instrumented
// exp2 exp10 : do not exist in quadmath (but counted)
// not instrumented at all
// frexp  ldexp (instrumentation discutable)
// all complex function

// sincos is a the sequence sin, cos: impact on counters

//shell LIST1="acos acosh asin asinh atan atanh cbrt erf exp exp2 exp10 expm1 log log10 log1p log2 tgamma lgamma sin sinh cos cosh sqrt tan tanh j0 j1 y0 y1"


enum Function1Name: size_t {
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
  enumexp10,
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

//shell LIST1COMPLEX="cexp clog cpow csqrt csin ccos ctan casin cacos catan csinh ccosh ctanh casinh cacosh catanh"
enum Function1NameComplex: size_t {
  //shell comand to generate:  for i in $LIST1COMPLEX ; do  echo "enum$i,"; done;
  enumcexp,
  enumclog,
  enumcpow,
  enumcsqrt,
  enumcsin,
  enumccos,
  enumctan,
  enumcasin,
  enumcacos,
  enumcatan,
  enumcsinh,
  enumccosh,
  enumctanh,
  enumcasinh,
  enumcacosh,
  enumcatanh,
  //fin shell
  enum_libm_function1_name_complex_size
};

enum Function2Name : size_t{
  enumatan2,
  enumhypot,
  enumpow,
  enumfdim,
  enum_libm_function2_name_size};


enum Function2IntFP : size_t{
  enumjn,
  enumyn,
  enum_libm_function2IntFP_name_size
};



enum Function3Name : size_t{
  enumfma,
  enum_libm_function3_name_size};


myLibMathFunction1* function1NameTab=NULL;
myLibMathFunction1Complex* function1NameComplexTab=NULL;
myLibMathFunction2* function2NameTab=NULL;
myLibMathFunction2IntFP* function2IntFPNameTab=NULL;
myLibMathFunction3* function3NameTab=NULL;


typedef enum FunctionParam: size_t{
			     oneReal=0,
			     twoReal,
			     threeReal,
			     intReal,
			     oneComplex,
			     enum_function_param_size
}FunctionParam_t;

size_t functionNameSize(size_t param){
  switch (param){
  case oneReal:
    return enum_libm_function1_name_size;
  case twoReal:
    return enum_libm_function2_name_size;
  case threeReal:
    return enum_libm_function3_name_size;
  case intReal:
    return enum_libm_function2IntFP_name_size;
  case oneComplex:
    return enum_libm_function1_name_complex_size;
  }
  return 0;
}


//Avoid libc allocator : not sur it is required 
static char bufferLibM[8000];
static size_t** data_=NULL;

class libMathCounter{
public:

  static void init(){
    if(data_==NULL){
      char* currentBuffer=bufferLibM;
      data_=(size_t**)currentBuffer;
      currentBuffer+= enum_function_param_size*sizeof(size_t*);
      for(size_t param=0; param < enum_function_param_size; param++){
	data_[param]=(size_t*)currentBuffer;
	currentBuffer+=(2*3 * functionNameSize(param)) * sizeof(size_t);
	for(size_t i=0 ; i< 2*3 * functionNameSize(param); i++){
	  data_[param][i]=0;
	}
      }
    }
  }

  static void finalyze(){}

  static void inc(size_t param, size_t enumLibm, size_t typeIndex, size_t instrumentStatus){
    data_[param][6*enumLibm + typeIndex*2 + instrumentStatus ]++;
  }

  static size_t getValue(size_t param, size_t enumLibm, size_t typeIndex, size_t instrumentStatus){
    return data_[param][6*enumLibm + typeIndex*2 + instrumentStatus ];
  }

};


unsigned int* cacheInstrumentStatus1=NULL;
unsigned int* cacheInstrumentStatus2=NULL;
unsigned int* cacheInstrumentStatus2IntFP=NULL;
unsigned int* cacheInstrumentStatus3=NULL;
unsigned int* cacheInstrumentStatus1Complex=NULL;

unsigned int* cacheNeedSeedUpdate=NULL;


template<class>
struct realTypeIndex;
template<>
struct realTypeIndex<float>{
  static const size_t index=0;
};
template<>
struct realTypeIndex<double>{
  static const size_t index=1;
};
template<>
struct realTypeIndex<long double>{
  static const size_t index=2;
};

template<>
struct realTypeIndex<float _Complex>{
  static const size_t index=0;
};
template<>
struct realTypeIndex<double _Complex>{
  static const size_t index=1;
};
template<>
struct realTypeIndex<long double _Complex>{
  static const size_t index=2;
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
void updateSeedCache(){
  if(*cacheNeedSeedUpdate == 0){
    uint64_t libm_seed=  VERROU_GET_SEED;
    vr_rand_setSeed_for_libm (&vr_rand, libm_seed);
    //    verrou_set_seed_for_libm (libm_seed);
    *cacheNeedSeedUpdate = 1;
  }
}
#endif


#ifndef INTERLIBM_STAND_ALONE
void generateExcludeSource(){
  for(size_t nbParam=0; nbParam <=enum_function_param_size; nbParam++){
    size_t paramSize= functionNameSize(nbParam);

    for(size_t i=0; i< paramSize;i++){
      std::string functionName;
      int line;
      if(nbParam==oneReal){
	functionName=function1NameTab[i].name();
	line=function1NameTab[i].getLine();
      }
      if(nbParam==twoReal){
	functionName=function2NameTab[i].name();
	line=function2NameTab[i].getLine();
      }
      if(nbParam==intReal){
	functionName=function2IntFPNameTab[i].name();
	line=function2IntFPNameTab[i].getLine();
      }
      if(nbParam==threeReal){
	functionName=function3NameTab[i].name();
	line=function3NameTab[i].getLine();
      }
      if(nbParam==oneComplex){
	functionName=function1NameComplexTab[i].name();
	line=function1NameComplexTab[i].getLine();
      }

      if( libMathCounter::getValue(nbParam,i,0,0)!=0){  //float
	std::string fctName=functionName+std::string("f");
	VERROU_GENERATE_EXCLUDE_SOURCE(fctName.c_str(), &line, fileName, libraryName);
      }
      if(libMathCounter::getValue(nbParam,i,1,0)!=0){ //double
	VERROU_GENERATE_EXCLUDE_SOURCE(functionName.c_str(), &line, fileName, libraryName);
      }
      if( libMathCounter::getValue(nbParam,i,2,0)!=0){//ldouble
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

  for(size_t nbParam=0; nbParam <=enum_function_param_size; nbParam++){
    size_t paramSize= functionNameSize(nbParam);

    for(size_t i=0; i< paramSize;i++){
      size_t total=0;
      size_t totalInst=0;
      for(size_t j=0;j<3;j++){
	total+=libMathCounter::getValue(nbParam,i,j,0)+libMathCounter::getValue(nbParam,i,j,1);
	totalInst+=libMathCounter::getValue(nbParam,i,j,0);
      }

      if(total!=0){
	std::cerr << "=="<<my_pid<<"== ";
	std::cerr<<  "---------------------------------------------------"<<std::endl;
	std::cerr << "=="<<my_pid<<"== ";

	if(nbParam==oneReal){
	  std::cerr<< function1NameTab[i].name();
	}
	if(nbParam==oneComplex){
	  std::cerr<< function1NameComplexTab[i].name();
	}
	if(nbParam==twoReal){
	  std::cerr<< function2NameTab[i].name();
	}
	if(nbParam==intReal){
	  std::cerr<< function2IntFPNameTab[i].name();
	}
	if(nbParam==threeReal){
	  std::cerr<< function3NameTab[i].name();
	}


	std::cerr<< "\t\t" <<  total << "\t" << totalInst<<std::endl;

	size_t totalFloat=libMathCounter::getValue(nbParam,i,0,0)+libMathCounter::getValue(nbParam,i,0,1);
	if(totalFloat>0){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " flt ";
	  std::cerr<< "\t" <<  totalFloat  << "\t" << libMathCounter::getValue(nbParam,i,0,0)<<std::endl;
	}

	size_t totalDouble=libMathCounter::getValue(nbParam,i,1,0)+libMathCounter::getValue(nbParam,i,1,1);
	if(totalDouble>0){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " dbl ";
	  std::cerr<< "\t" <<  totalDouble  << "\t" << libMathCounter::getValue(nbParam,i,1,0)<<std::endl;
	}

	size_t totalLDouble= libMathCounter::getValue(nbParam,i,2,0)+libMathCounter::getValue(nbParam,i,2,1);
	if(totalLDouble){
	  std::cerr << "=="<<my_pid<<"== ";
	  std::cerr<< " `-" " lgd ";
	  std::cerr<< "\t" << totalLDouble  << "\t" << libMathCounter::getValue(nbParam,i,2,0)<<std::endl;
	}
      }
    }
  }
}


#ifndef INTERLIBM_STAND_ALONE
static __float128 verrou_libm_res_ref=0.;
#endif


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
#ifndef INTERLIBM_STAND_ALONE
    verrou_libm_res_ref=ref;
#endif
    return (RealType)ref;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
#ifdef INTERLIBM_STAND_ALONE
    const __float128 a(p.arg1);
    __float128 ref=LIBMQ::apply(a);
#else
    __float128 ref=verrou_libm_res_ref;
#endif
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
    if(  LIBMQ::getParity()==enumEven ){
      if(p.arg1>=0){
	return r.hash(p, hashOp);
      }else{
	const RealType absp1(-p.arg1);
	const PackArgs pnew(absp1);
	return r.hash(pnew, hashOp);
      }
    }
    if(  LIBMQ::getParity()==enumOdd ){
      if(p.arg1>0){
	return r.hash(p, hashOp);
      }else{
	const RealType absp1(-p.arg1);
	const PackArgs pnew(absp1);
	return r.hashBar(pnew, hashOp);
      }
    }
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

  template<class RANDCOM>
  static inline typename RANDCOM::TypeOut hashCom(const RANDCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    if( LIBMQ::isCommutative()){
      const RealType pmin(std::min<RealType>(p.arg1,p.arg2));
      const RealType pmax(std::max<RealType>(p.arg1,p.arg2));
      const vr_packArg<RealType,2> pcom(pmin,pmax);
      return r.hash(pcom,hashOp);
    }
    return r.hash(p,hashOp);
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    if( LIBMQ::getParity2()==  enum2Even  ){
      const RealType p1= p.arg1>=0?p.arg1:-p.arg1;
      const RealType p2= p.arg2>=0?p.arg2:-p.arg2;
      if( LIBMQ::isCommutative()){
	const RealType pmin(std::min<RealType>(p1,p2));
	const RealType pmax(std::max<RealType>(p1,p2));
	const vr_packArg<RealType,2> pcom(pmin,pmax);
	return r.hash(pcom,hashOp);
      }else{
	const vr_packArg<RealType,2> pcom(p1,p2);
	return r.hash(pcom,hashOp);
      }
    }
    if( LIBMQ::getParity2()==  enum2OddFirst ){
      if(p.arg1>0){
	return r.hash(p, hashOp);
      }else{
	const RealType absp1(-p.arg1);
	const RealType p2(p.arg2);
	const PackArgs pnew(absp1, p2);
	return r.hashBar(pnew, hashOp);
      }
    }
    return r.hash(p, hashOp);
  };
};


template<class LIBMQ, typename REALTYPE >
class libMathFunction2IntFP{
public:
  typedef REALTYPE RealType;
  typedef packargsIntReal<REALTYPE> PackArgs;
  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "libmath IntFP";}
  static inline uint32_t getHash(){return LIBMQ::getHash();}

  static inline RealType nearestOp (const PackArgs& p) {
    const int & a(p.arg1);
    const __float128  b(p.arg2);
    __float128 ref=LIBMQ::apply(a,b);
    return (RealType)ref;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {\
    const int & a(p.arg1);
    const __float128  b(p.arg2);
    __float128 ref=LIBMQ::apply(a,b);
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

  template<class RANDCOM>
  static inline typename RANDCOM::TypeOut hashCom(const RANDCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    return r.hash(p,hashOp);
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(getHash());
    return r.hash(p,hashOp);
  };

};



/*Warning no used : for fma we use MAddOp from vr_op.hxx : if you want to use it please pay attention to hash* methods */
template<class LIBMQ, typename REALTYPE >
class libMathFunction3{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,3> PackArgs;
  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "libmath3";}
  static inline uint32_t getHash(){return LIBMQ::getHash();}

  static inline RealType nearestOp (const PackArgs& p) {
    const __float128 a=p.arg1;
    const __float128 b=p.arg2;
    const __float128 c=p.arg3;
    __float128 ref=LIBMQ::apply(a, b, c);
    return (RealType)ref;
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {\
    const __float128 a=p.arg1;
    const __float128 b=p.arg2;
    const __float128 c=p.arg3;

    __float128 ref=LIBMQ::apply(a, b, c);
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

  template<class RANDCOM>
  static inline typename RANDCOM::TypeOut hashCom(const RANDCOM& r,const PackArgs& p){
    const RealType pmin(std::min<RealType>(p.arg1,p.arg2));
    const RealType pmax(std::max<RealType>(p.arg1,p.arg2));
    const vr_packArg<RealType,3> pcom(pmin,pmax,p.arg3);
    const uint32_t hashOp(getHash());
    return r.hash(pcom,hashOp);
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    return hashCom(r,p);
  };

};


#ifdef INTERLIBM_STAND_ALONE
bool isInstrumented1(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
bool isInstrumented2(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
bool isInstrumented2IntFP(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
bool isInstrumented3(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  return true;
}
#else
bool isInstrumented1(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  unsigned int index= functionType * enum_libm_function1_name_size+ functionEnum;
  int line=( function1NameTab[functionEnum]).getLine();
  if(  cacheInstrumentStatus1[index]==0){
    if(VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(functionName, &line, fileName, libraryName)){
      cacheInstrumentStatus1[index]=1;
    }else{
      cacheInstrumentStatus1[index]=2;
    }
  }
  if( cacheInstrumentStatus1[index]==1){
    updateSeedCache();
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
    updateSeedCache();
    return true;
  }else{
    return false;
  }
}

bool isInstrumented2IntFP(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  unsigned int index= functionType * enum_libm_function2IntFP_name_size+ functionEnum;
  int line=( function2IntFPNameTab[functionEnum]).getLine();
  if(  cacheInstrumentStatus2IntFP[index]==0){
    if(VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(functionName, &line, fileName, libraryName)){
      cacheInstrumentStatus2IntFP[index]=1;
    }else{
      cacheInstrumentStatus2IntFP[index]=2;
    }
  }
  if( cacheInstrumentStatus2IntFP[index]==1){
    updateSeedCache();
    return true;
  }else{
    return false;
  }
}

bool isInstrumented3(const char* functionName, unsigned int functionEnum, unsigned int functionType){
  unsigned int index= functionType * enum_libm_function3_name_size+ functionEnum;
  int line=( function3NameTab[functionEnum]).getLine();
  if(  cacheInstrumentStatus3[index]==0){
    if(VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(functionName, &line, fileName, libraryName)){
      cacheInstrumentStatus3[index]=1;
    }else{
      cacheInstrumentStatus3[index]=2;
    }
  }
  if( cacheInstrumentStatus3[index]==1){
    updateSeedCache();
    return true;
  }else{
    return false;
  }
}
#endif


template<class REALTYPE, size_t NBPARAM, class LIBMQ, int ENUM_LIBM >
class instrumentFunction{
public:
  template<class LIBMNAMETAB, class PACKARGS>
  static REALTYPE apply(const LIBMNAMETAB& functionNameTab,const char* fctStr, bool isInstrumented,  const PACKARGS& p){
    if(isInstrumented){
      libMathCounter::inc(NBPARAM,ENUM_LIBM, realTypeIndex<REALTYPE>::index,0);
    }else{
      libMathCounter::inc(NBPARAM,ENUM_LIBM, realTypeIndex<REALTYPE>::index,1);
    }
    if(ROUNDINGMODE==VR_NATIVE || (!(isInstrumented) && ROUNDINGMODE_NOINST==VR_NATIVE)){
      return functionNameTab[ENUM_LIBM].apply(p);
    }
    if(ROUNDINGMODE==VR_NEAREST || (!(isInstrumented) && ROUNDINGMODE_NOINST==VR_NEAREST)){
      typedef OpWithNearestRoundingMode<LIBMQ > Op;
      REALTYPE res;
      Op::apply(p,&res,NULL);
      return res;
    }
    typedef OpWithDynSelectedRoundingMode<LIBMQ > Op;
    REALTYPE res;
    Op::apply(p,&res,NULL);
    return res;
  }
};


#define STRINGIFY(A) #A

#define DEFINE_INTERP_LIBM1_C_IMPL(FCT)\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a){return FCT##q(a);}		\
    static __uint64_t getHash(){return enum##FCT; }                     \
    static libMparity_t getParity(){return function1NameTab[enum##FCT].getParity();}; \
};									\
  extern "C"{								\
  double FCT (double a){						\
    const char fctStr[]=STRINGIFY(FCT);					\
    const vr_packArg<double,1> p(a);					\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented1(fctStr, enum##FCT,1); \
    typedef instrumentFunction<double,oneReal, libMathFunction1<libmq##FCT,double>, enum##FCT > inst; \
    return inst::apply(function1NameTab, fctStr, isInstrumented, p);	\
  }									\
									\
  float FCT##f (float a){						\
    char fctStr[]=STRINGIFY(FCT##f);					\
    const vr_packArg<float,1> p(a);					\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented1(fctStr, enum##FCT,0); \
    typedef instrumentFunction<float,oneReal, libMathFunction1<libmq##FCT,float>, enum##FCT > inst; \
    return inst::apply(function1NameTab, fctStr, isInstrumented, p);	\
  }									\
									\
  long double FCT##l (long double a){					\
    libMathCounter::inc(oneReal,enum##FCT, realTypeIndex<long double>::index,1);\
    return function1NameTab[enum##FCT].apply(a);			\
  }									\
};

#define DEFINE_INTERP_LIBM1_C_IMPL_UNINST(FCT)				\
  extern "C"{								\
    float FCT##f (float a){						\
      libMathCounter::inc(oneReal,enum##FCT, realTypeIndex<float>::index,1); \
      return function1NameTab[enum##FCT].apply(a);			\
    }									\
    double FCT (double a){						\
      libMathCounter::inc(oneReal,enum##FCT, realTypeIndex<double>::index,1); \
      return function1NameTab[enum##FCT].apply(a);			\
    }									\
    long double FCT##l (long double a){				\
      libMathCounter::inc(oneReal,enum##FCT, realTypeIndex<long double>::index,1); \
      return function1NameTab[enum##FCT].apply(a);			\
    }									\
  };

#define DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(FCT)			\
  extern "C"{								\
    float _Complex FCT##f (float _Complex a){				\
      libMathCounter::inc(oneComplex,enum##FCT, realTypeIndex<float>::index,1); \
      return function1NameComplexTab[enum##FCT].apply(a);		\
    }									\
    double _Complex FCT (double _Complex a){				\
      libMathCounter::inc(oneComplex,enum##FCT, realTypeIndex<double>::index,1); \
      return function1NameComplexTab[enum##FCT].apply(a);		\
    }									\
    long double _Complex FCT##l (long double _Complex a){		\
      libMathCounter::inc(oneComplex,enum##FCT, realTypeIndex<long double>::index,1); \
      return function1NameComplexTab[enum##FCT].apply(a);		\
    }									\
  };


#define DEFINE_INTERP_LIBM2_C_IMPL(FCT)				\
  struct libmq##FCT{							\
    static __float128 apply(__float128 a,__float128 b){return FCT##q(a,b);} \
    static __uint64_t getHash(){return enum##FCT; }			\
    static bool isCommutative(){return function2NameTab[enum##FCT].getCommutativity();}\
    static libM2parity_t getParity2(){return function2NameTab[enum##FCT].getParity2();}; \
  };									\
  extern "C"{								\
    double FCT (double a, double b){					\
      const char fctStr[]=STRINGIFY(FCT);				\
      const vr_packArg<double,2> p(a,b);				\
      const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented2(fctStr, enum##FCT,1); \
      typedef instrumentFunction<double,twoReal, libMathFunction2<libmq##FCT,double>, enum##FCT > inst; \
      return inst::apply(function2NameTab, fctStr, isInstrumented, p);	\
    }									\
									\
    float FCT##f (float a, float b){					\
    const char fctStr[]=STRINGIFY(FCT##f);				\
      const vr_packArg<float,2> p(a,b);					\
      const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented2(fctStr, enum##FCT,0); \
      typedef instrumentFunction<float,twoReal, libMathFunction2<libmq##FCT,float>, enum##FCT > inst; \
      return inst::apply(function2NameTab, fctStr, isInstrumented, p);	\
    }									\
									\
    long double FCT##l (long double a, long double b){			\
      libMathCounter::inc(twoReal,enum##FCT, realTypeIndex<long double>::index,1); \
      return function2NameTab[enum##FCT].apply(a,b);			\
    }									\
  };

#define DEFINE_INTERP_LIBM2INTFP_C_IMPL(FCT)				\
  struct libmq##FCT{							\
    static __float128 apply(int a,__float128 b){		       \
      WRITE_DEBUG("befor FCTq\n");					\
__float128 res=FCT##q(a,b);						\
      WRITE_DEBUG("after FCTq\n");					\
      return res;							\
    }									\
    static __uint64_t getHash(){return enum##FCT; }			\
  };									\
  extern "C"{								\
  double FCT (int a, double b){						\
  WRITE_DEBUG("lib double 1\n");						\
  const packargsIntReal<double> p((double)a,b);				\
    const char fctStr[]=#FCT;						\
    const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented2IntFP(fctStr, enum##FCT,1); \
    typedef instrumentFunction<double,intReal, libMathFunction2IntFP<libmq##FCT,double>, enum##FCT > inst; \
    return inst::apply(function2IntFPNameTab, fctStr, isInstrumented, p);	\
  };									\
float FCT##f (int a, float b){						\
  const packargsIntReal<float> p((float)a,b);				\
  const char fctStr[]=STRINGIFY(FCT##f);				\
  const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented2IntFP(fctStr, enum##FCT,0); \
  typedef instrumentFunction<float,intReal, libMathFunction2IntFP<libmq##FCT,float>, enum##FCT > inst; \
  return inst::apply(function2IntFPNameTab, fctStr, isInstrumented, p);	\
  }									\
									\
   long double FCT##l (int a, long double b){				\
     libMathCounter::inc(intReal,enum##FCT, realTypeIndex<long double>::index,1); \
     return function2IntFPNameTab[enum##FCT].apply(a,b);		\
   }									\
  };




#define DEFINE_INTERP_LIBM3_FMA_C_IMPL(FCT)				\
  extern "C"{								\
    double FCT (double a, double b, double c){					\
      const vr_packArg<double,3> p(a,b,c);				\
      const char fctStr[]=#FCT;						\
      const bool isInstrumented=VERROU_IS_INSTRUMENTED_DOUBLE && isInstrumented3(fctStr, enum##FCT,1); \
      typedef instrumentFunction<double,threeReal, MAddOp<double>, enum##FCT > inst; \
      return inst::apply(function3NameTab, fctStr, isInstrumented, p); \
    }									\
									\
    float FCT##f (float a, float b, float c){				\
      const vr_packArg<float,3> p(a,b,c);				\
      const char fctStr[]=STRINGIFY(FCT##f);				\
      const bool isInstrumented=VERROU_IS_INSTRUMENTED_FLOAT && isInstrumented3(fctStr, enum##FCT,0); \
      typedef instrumentFunction<float,threeReal, MAddOp<float>, enum##FCT > inst; \
      return inst::apply(function3NameTab, fctStr, isInstrumented, p); \
      }									\
									\
    long double FCT##l (long double a, long double b, long double c){	\
     libMathCounter::inc(threeReal,enum##FCT, realTypeIndex<long double>::index,1); \
      return function3NameTab[enum##FCT].apply(a,b,c);			\
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
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST(exp2);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST(exp10);
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
#undef DEFINE_INTERP_LIBM1_C_IMPL

//shell for i in $LIST1COMPLEX ; do  echo " DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX($i);"; done;
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(cexp);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(clog);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(cpow);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(csqrt);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(csin);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(ccos);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(ctan);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(casin);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(cacos);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(catan);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(csinh);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(ccosh);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(ctanh);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(casinh);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(cacosh);
 DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX(catanh);
#undef DEFINE_INTERP_LIBM1_C_IMPL_UNINST_COMPLEX

DEFINE_INTERP_LIBM2_C_IMPL(atan2);
DEFINE_INTERP_LIBM2_C_IMPL(hypot);
DEFINE_INTERP_LIBM2_C_IMPL(pow);
DEFINE_INTERP_LIBM2_C_IMPL(fdim);
#undef DEFINE_INTERP_LIBM2_C_IMPL

DEFINE_INTERP_LIBM2INTFP_C_IMPL(yn);
DEFINE_INTERP_LIBM2INTFP_C_IMPL(jn);

#undef DEFINE_INTERP_LIBM2INTFP_C_IMPL

DEFINE_INTERP_LIBM3_FMA_C_IMPL(fma);
#undef DEFINE_INTERP_LIB3_FMA_C_IMPL

void sincos(double x, double* resSin, double* resCos){
  *resSin=sin(x);
  *resCos=cos(x);
}
void sincosf(float x, float* resSin, float* resCos){
  *resSin=sin(x);
  *resCos=cos(x);
}
void sincosl(long double x, long double* resSin, long double* resCos){
  *resSin=sinl(x);
  *resCos=cosl(x);
}



vr_RoundingMode stringToRoundingMode(std::string strEnv){
  if(strEnv==std::string("random")){
      return VR_RANDOM;
    }
    if(strEnv==std::string("random_det")){
      return VR_RANDOM_DET;
    }
    if(strEnv==std::string("random_comdet")){
      return VR_RANDOM_COMDET;
    }
    if(strEnv==std::string("random_scomdet")){
      return VR_RANDOM_SCOMDET;
    }
    if(strEnv==std::string("sr_monotonic")){
      return VR_SR_MONOTONIC;
    }
    if(strEnv==std::string("sr_smonotonic")){
      return VR_SR_SMONOTONIC;
    }
    if(strEnv==std::string("average")){
      return VR_AVERAGE;
    }
    if(strEnv==std::string("average_det")){
      return VR_AVERAGE_DET;
    }
    if(strEnv==std::string("average_comdet")){
      return VR_AVERAGE_COMDET;
    }
    if(strEnv==std::string("average_scomdet")){
      return VR_AVERAGE_SCOMDET;
    }
    if(strEnv==std::string("nearest")){
      return VR_NEAREST;
    }
    if(strEnv==std::string("upward")){
      return VR_UPWARD;
    }
    if(strEnv==std::string("downward")){
      return VR_DOWNWARD;
    }
    if(strEnv==std::string("toward_zero")){
      return VR_ZERO;
    }
    if(strEnv==std::string("away_zero")){
      return VR_AWAY_ZERO;
    }
    if(strEnv==std::string("farthest")){
      return VR_FARTHEST;
    }
    if(strEnv==std::string("float")){
      return VR_FLOAT;
    }
    if(strEnv==std::string("native")){
      return VR_NATIVE;
    }
    if(strEnv==std::string("ftz")){
      WRITE_DEBUG("Rounding VR_FTZ not yet implemented in interlibmath");
      exit(1);
    }
    WRITE_DEBUG("Unknown rounding mod in interlibmath");
    exit(1);
}



void __attribute__((constructor)) init_interlibmath(){
  my_pid = getpid();
  ROUNDINGMODE = VERROU_GET_LIBM_ROUNDING; // VR_NATIVE; //Default value
  ROUNDINGMODE_NOINST = VERROU_GET_LIBM_ROUNDING_NO_INST;


  libMathCounter::init();
  if(function2IntFPNameTab==NULL){
    function2IntFPNameTab=(myLibMathFunction2IntFP*)malloc(enum_libm_function2IntFP_name_size* sizeof(  myLibMathFunction2IntFP)) ;
    if( function2IntFPNameTab==NULL){
      WRITE_DEBUG("malloc ERROR\n");
    }

    function2IntFPNameTab[enumjn]=myLibMathFunction2IntFP("jn",enumjn, __LINE__);
    function2IntFPNameTab[enumyn]=myLibMathFunction2IntFP("yn",enumyn, __LINE__);
  }
  if(function1NameTab==NULL){
    function1NameTab=(myLibMathFunction1*)malloc(enum_libm_function1_name_size* sizeof(  myLibMathFunction1)) ;
    if( function1NameTab==NULL){
      WRITE_DEBUG("malloc ERROR\n");
    }
    //    for i in $LIST1 ; do  echo "function1NameTab[enum$i]=myLibMathFunction1(\"$i\", enum$i,__LINE__),"; done;
    function1NameTab[enumacos]=myLibMathFunction1("acos", enumacos,__LINE__);
    function1NameTab[enumacosh]=myLibMathFunction1("acosh", enumacosh,__LINE__);
    function1NameTab[enumasin]=myLibMathFunction1("asin", enumasin,__LINE__);
    function1NameTab[enumasinh]=myLibMathFunction1("asinh", enumasinh,__LINE__);
    function1NameTab[enumatan]=myLibMathFunction1("atan", enumatan,__LINE__);
    function1NameTab[enumatanh]=myLibMathFunction1("atanh", enumatanh,__LINE__);
    function1NameTab[enumcbrt]=myLibMathFunction1("cbrt", enumcbrt,__LINE__);
    function1NameTab[enumerf]=myLibMathFunction1("erf", enumerf,__LINE__);
    function1NameTab[enumexp]=myLibMathFunction1("exp", enumexp,__LINE__);
    function1NameTab[enumexp2]=myLibMathFunction1("exp2", enumexp2,__LINE__);
    function1NameTab[enumexp10]=myLibMathFunction1("exp10", enumexp10,__LINE__);
    function1NameTab[enumexpm1]=myLibMathFunction1("expm1", enumexpm1,__LINE__);
    function1NameTab[enumlog]=myLibMathFunction1("log", enumlog,__LINE__);
    function1NameTab[enumlog10]=myLibMathFunction1("log10", enumlog10,__LINE__);
    function1NameTab[enumlog1p]=myLibMathFunction1("log1p", enumlog1p,__LINE__);
    function1NameTab[enumlog2]=myLibMathFunction1("log2", enumlog2,__LINE__);
    function1NameTab[enumtgamma]=myLibMathFunction1("tgamma", enumtgamma,__LINE__);
    function1NameTab[enumlgamma]=myLibMathFunction1("lgamma", enumlgamma,__LINE__);
    function1NameTab[enumsin]=myLibMathFunction1("sin", enumsin,__LINE__);
    function1NameTab[enumsinh]=myLibMathFunction1("sinh", enumsinh,__LINE__);
    function1NameTab[enumcos]=myLibMathFunction1("cos", enumcos,__LINE__);
    function1NameTab[enumcosh]=myLibMathFunction1("cosh", enumcosh,__LINE__);
    function1NameTab[enumsqrt]=myLibMathFunction1("sqrt", enumsqrt,__LINE__);
    function1NameTab[enumtan]=myLibMathFunction1("tan", enumtan,__LINE__);
    function1NameTab[enumtanh]=myLibMathFunction1("tanh", enumtanh,__LINE__);
    function1NameTab[enumj0]=myLibMathFunction1("j0", enumj0,__LINE__);
    function1NameTab[enumj1]=myLibMathFunction1("j1", enumj1,__LINE__);
    function1NameTab[enumy0]=myLibMathFunction1("y0", enumy0,__LINE__);
    function1NameTab[enumy1]=myLibMathFunction1("y1", enumy1,__LINE__);

    for(int i=0; i< (int)enum_libm_function1_name_size;i++){
      function1NameTab[i].setParity(enumNoParity);
    }
    //set ODD function
    function1NameTab[enumsin].setParity(enumOdd);
    function1NameTab[enumsinh].setParity(enumOdd);
    function1NameTab[enumasin].setParity(enumOdd);
    function1NameTab[enumasinh].setParity(enumOdd);
    function1NameTab[enumtan].setParity(enumOdd);
    function1NameTab[enumtanh].setParity(enumOdd);
    function1NameTab[enumatan].setParity(enumOdd);
    function1NameTab[enumatanh].setParity(enumOdd);
    function1NameTab[enumcbrt].setParity(enumOdd);
    function1NameTab[enumerf].setParity(enumOdd);
    function1NameTab[enumj1].setParity(enumOdd);

    //set EVEN function
    function1NameTab[enumcos].setParity(enumEven);
    function1NameTab[enumcosh].setParity(enumEven);
    function1NameTab[enumj0].setParity(enumEven);
  }

  if(function1NameComplexTab==NULL){
    function1NameComplexTab=(myLibMathFunction1Complex*)malloc(enum_libm_function1_name_complex_size* sizeof(  myLibMathFunction1Complex));
    if( function1NameComplexTab==NULL){
      WRITE_DEBUG("malloc ERROR\n");
    }
    //    for i in $LIST1COMPLEX ; do  echo "function1NameComplexTab[enum$i] = myLibMathFunction1Complex(\"$i\", enum$i,__LINE__);"; done;
    function1NameComplexTab[enumcexp] = myLibMathFunction1Complex("cexp", enumcexp,__LINE__);
    function1NameComplexTab[enumclog] = myLibMathFunction1Complex("clog", enumclog,__LINE__);
    function1NameComplexTab[enumcpow] = myLibMathFunction1Complex("cpow", enumcpow,__LINE__);
    function1NameComplexTab[enumcsqrt] = myLibMathFunction1Complex("csqrt", enumcsqrt,__LINE__);
    function1NameComplexTab[enumcsin] = myLibMathFunction1Complex("csin", enumcsin,__LINE__);
    function1NameComplexTab[enumccos] = myLibMathFunction1Complex("ccos", enumccos,__LINE__);
    function1NameComplexTab[enumctan] = myLibMathFunction1Complex("ctan", enumctan,__LINE__);
    function1NameComplexTab[enumcasin] = myLibMathFunction1Complex("casin", enumcasin,__LINE__);
    function1NameComplexTab[enumcacos] = myLibMathFunction1Complex("cacos", enumcacos,__LINE__);
    function1NameComplexTab[enumcatan] = myLibMathFunction1Complex("catan", enumcatan,__LINE__);
    function1NameComplexTab[enumcsinh] = myLibMathFunction1Complex("csinh", enumcsinh,__LINE__);
    function1NameComplexTab[enumccosh] = myLibMathFunction1Complex("ccosh", enumccosh,__LINE__);
    function1NameComplexTab[enumctanh] = myLibMathFunction1Complex("ctanh", enumctanh,__LINE__);
    function1NameComplexTab[enumcasinh] = myLibMathFunction1Complex("casinh", enumcasinh,__LINE__);
    function1NameComplexTab[enumcacosh] = myLibMathFunction1Complex("cacosh", enumcacosh,__LINE__);
    function1NameComplexTab[enumcatanh] = myLibMathFunction1Complex("catanh", enumcatanh,__LINE__);
  }

  if(function2NameTab==NULL){
    function2NameTab=(myLibMathFunction2*)malloc(enum_libm_function2_name_size* sizeof(  myLibMathFunction2)) ;
    if( function2NameTab==NULL){
      WRITE_DEBUG("malloc ERROR\n");
    }
    function2NameTab[enumatan2] = myLibMathFunction2("atan2",enumatan2, __LINE__);
    function2NameTab[enumhypot] = myLibMathFunction2("hypot", enumhypot, __LINE__);
    function2NameTab[enumpow]   = myLibMathFunction2("pow", enumpow, __LINE__);
    function2NameTab[enumfdim]  = myLibMathFunction2("fdim",enumfdim, __LINE__);

    for(int i=0; i< (int)enum_libm_function2_name_size;i++){
      function2NameTab[i].setCommutativity(false);
      function2NameTab[i].setParity2(enum2NoParity);
    }
    function2NameTab[enumhypot].setCommutativity(true);
    function2NameTab[enumhypot].setParity2(enum2Even);
    function2NameTab[enumatan2].setParity2(enum2OddFirst);
  }
  if(function3NameTab==NULL){
    function3NameTab=(myLibMathFunction3*)malloc(enum_libm_function3_name_size* sizeof(  myLibMathFunction3)) ;
    if( function3NameTab==NULL){
      WRITE_DEBUG("malloc ERROR\n");
    }
    function3NameTab[enumfma]=myLibMathFunction3("fma",enumfma, __LINE__);
  };

#ifdef INTERLIBM_STAND_ALONE
  struct timeval now;
  gettimeofday(&now, NULL);
  uint64_t vr_seed=  now.tv_usec + my_pid;
  vr_rand_setSeed(&vr_rand, vr_seed);

  char* vrm=std::getenv("VERROU_LIBM_ROUNDING_MODE");
  if(vrm==NULL){
    vrm=std::getenv("VERROU_ROUNDING_MODE");
  }

  if(vrm!=NULL){
    std::string envString(vrm);
    ROUNDINGMODE=stringToRoundingMode(envString);
  }
#else

  cacheInstrumentStatus1=(unsigned int*)calloc(3 * enum_libm_function1_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus1, 3 * enum_libm_function1_name_size * sizeof(unsigned int));

  cacheInstrumentStatus2=(unsigned int*)calloc(3 * enum_libm_function2_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus2, 3 * enum_libm_function2_name_size * sizeof(unsigned int));

  cacheInstrumentStatus2IntFP=(unsigned int*)calloc(3 * enum_libm_function2IntFP_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus2IntFP, 3 * enum_libm_function2IntFP_name_size * sizeof(unsigned int));

  cacheInstrumentStatus3=(unsigned int*)calloc(3 * enum_libm_function3_name_size, sizeof(unsigned int));
  VERROU_REGISTER_CACHE(cacheInstrumentStatus3, 3 * enum_libm_function3_name_size * sizeof(unsigned int));

  cacheNeedSeedUpdate=(unsigned int*)calloc(1, sizeof(unsigned int));
  VERROU_REGISTER_CACHE_SEED(cacheNeedSeedUpdate);
#endif


  WRITE_DEBUG("fini init\n");
}


void __attribute__((destructor)) finalyze_interlibmath(){
#ifndef INTERLIBM_STAND_ALONE
  generateExcludeSource();
#endif
  if(VERROU_COUNT_OP){
    printCounter();
  }
  libMathCounter::finalyze();

  for(size_t i=0; i <enum_libm_function1_name_size;i++){
    function1NameTab[i].liberate();
  }
  free(function1NameTab);

  for(size_t i=0; i <enum_libm_function1_name_complex_size;i++){
    function1NameComplexTab[i].liberate();
  }
  free(function1NameComplexTab);

  for(size_t i=0; i <enum_libm_function2_name_size;i++){
    function2NameTab[i].liberate();
  }
  free(function2NameTab);

  for(size_t i=0; i <enum_libm_function2IntFP_name_size;i++){
    function2IntFPNameTab[i].liberate();
  }
  free(function2IntFPNameTab);

  for(size_t i=0; i <enum_libm_function3_name_size;i++){
    function3NameTab[i].liberate();
  }
  free(function3NameTab);

#ifndef INTERLIBM_STAND_ALONE
  free(cacheInstrumentStatus1);
  free(cacheInstrumentStatus2);
  free(cacheInstrumentStatus2IntFP);
  free(cacheInstrumentStatus3);
  free(cacheNeedSeedUpdate);
#endif

};
