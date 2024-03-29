/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU Lesser General Public License is contained in the file COPYING.
*/



#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string.h>

#include <fenv.h>
#include <limits>

#include "verrou.h"

#ifdef  TEST_FMA
#ifdef __x86_64__
#include  <immintrin.h>
#endif
#ifdef __aarch64__
#include <arm_neon.h>
#endif
#endif

#ifdef TEST_SSE
#ifdef __x86_64__
#include  <immintrin.h>
#endif
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


template<class REAL> std::string typeName(){
  return std::string("unknown");
}

template<>
std::string typeName<long double>(){
  return std::string("long double");
}

template<>
std::string typeName<double>(){
  return std::string("double");
}
template<>
std::string typeName<float>(){
  return std::string("float");
}



template<class REALTYPE, class REALTYPEREF=REALTYPE>
class test{
public:
  test(REALTYPEREF a):expectedResult(a){
  }
  
  REALTYPEREF res;
  REALTYPEREF expectedResult;
  void check(){
    std::cout.precision(16);
    std::cout << name()<<"<"<< typeName<REALTYPE>()<<">" <<":\tres: " << res
	      << "\ttheo: "<< expectedResult
	      << "\tdiff: "<<  res-expectedResult<<std::endl;
    
  }

  void run(){
    startInst(fenv,roundingMode);    
    res=(REALTYPEREF)compute();
    stopInst(fenv,roundingMode);
    check();
  }


  virtual REALTYPE compute()=0;
  virtual std::string name()=0;
};



template<class REALTYPE>
class testInc0d1: public test<REALTYPE>{
 public:
  testInc0d1():test<REALTYPE>(100001.),
	  size(1000000),
	  step(0.1),
	  init(1.)
    {
    }

  std::string name(){
    return std::string("testInc0d1");
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
class testInc0d1m: public test<REALTYPE>{
 public:
  testInc0d1m():test<REALTYPE>(-100001.),
    size(1000000),
    step(-0.1),
    init(-1.)
    {
    }

  std::string name(){
    return std::string("testInc0d1m");
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
class testIncSquare0d1: public test<REALTYPE>{
 public:
  testIncSquare0d1():test<REALTYPE>(10000),
	  size(1000000),
	  step(0.1),
	  init(0.)
    {

    }

  std::string name(){
    return std::string("testIncSquare0d1");
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
class testIncSquare0d1m: public test<REALTYPE>{
 public:
  testIncSquare0d1m():test<REALTYPE>(-10000),
	  size(1000000),
	  step(-0.1),
	  init(0.)
    {

    }

  std::string name(){
    return std::string("testIncSquare0d1m");
  }


  REALTYPE compute(){    
    REALTYPE acc=init;
    for(int i=0; i<size; i++){
      acc+=(-(REALTYPE)1.0*step)*step;
    }  
    return acc;
  }
  
 private:
  const int size;
  const REALTYPE step;
  const REALTYPE init;
};



template<class REALTYPE>
class testIncDiv10:public test<REALTYPE>{
 public:
  testIncDiv10():test<REALTYPE>(100000),
	  size(1000000),
	  stepDiv(10.),
	  init(0.)
    {
    }

  std::string name(){
    return std::string("testIncDiv10");
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
class testIncDiv10m:public test<REALTYPE>{
 public:
  testIncDiv10m():test<REALTYPE>(-100000),
    size(1000000),
    stepDiv(-10.),
    init(0.)
  {
  }

  std::string name(){
    return std::string("testIncDiv10m");
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
class testInvariantProdDiv:public test<REALTYPE>{
 public:

  //The size are adapted to avoid inf
  static int getSize(long double a){ return 150;};
  static int getSize(double a){ return 150;};
  static int getSize(float a){ return 34;} ;
  static int getSize(){return getSize((REALTYPE)0.);};
  
  testInvariantProdDiv():test<REALTYPE>(1.),
	  size(getSize()),
	  init(1.){}

  std::string name(){
    return std::string("testInvariantProdDiv");
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
class testInvariantProdDivm:public test<REALTYPE>{
 public:

  //The size are adapted to avoid inf
  static int getSize(long double a){ return 150;};
  static int getSize(double a){ return 150;};
  static int getSize(float a){ return 34;} ;
  static int getSize(){return getSize((REALTYPE)0.);};
  
  testInvariantProdDivm():test<REALTYPE>(-1.),
	  size(getSize()),
	  init(-1.){}

  std::string name(){
    return std::string("testInvariantProdDivm");
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





  inline double myFma(const double& a, const double& b, const double& c){
    double d;
#ifdef TEST_FMA
#if defined(__X86_64__)
    __m128d ai, bi,ci,di;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmadd_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);
#elif  defined(__aarch64__)
  const float64x1_t ai=vld1_f64(&a);
  const float64x1_t bi=vld1_f64(&b);
  const float64x1_t ci=vld1_f64(&c);
  const float64x1_t di=vfma_f64(ci,ai,bi);// warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  vst1_f64(&d, di);
#else
#error "notyet implemented for this architecture"
#endif
#else
    d=a*b+c;
#endif
    return d;
  }


  inline float myFma(const float& a, const float& b, const float& c){
    float d;
#ifdef TEST_FMA
#if defined(__X86_64__)
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    bi = _mm_load_ss(&b);
    ci = _mm_load_ss(&c);
    di=_mm_fmadd_ss(ai,bi,ci);
    d=_mm_cvtss_f32(di);
#elif  defined(__aarch64__)
  float av[2]={a,0};
  float bv[2]={b,0};
  float cv[2]={c,0};

  float32x2_t ap=vld1_f32(av);
  float32x2_t bp=vld1_f32(bv);
  float32x2_t cp=vld1_f32(cv);

  float32x2_t resp= vfma_f32(cp,ap,bp); // warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  float res[2];
  vst1_f32(res, resp);
  d=res[0];
#else
#error "notyet implemented for this architecture"
#endif
#else
    d=a*b+c;
#endif
    return d;
  }



template<class REALTYPE>
class testFma:public test<REALTYPE>{
 public:
  
  testFma():test<REALTYPE>(10000),
    size(1000000),
    value(0.1),
    init(0.){}

  std::string name(){
    return std::string("testFma");
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


template<class REALTYPE>
class testFmam:public test<REALTYPE>{
 public:
  
  testFmam():test<REALTYPE>(-10000),
    size(1000000),
    value(-0.1),
    init(0.){}

  std::string name(){
    return std::string("testFmam");
  }


  REALTYPE compute(){    
    REALTYPE acc=init;

    for(int i=0; i<size; i++){
      acc=myFma(-value,value,acc);
    } 
    return acc;
  }
  
 private:
  const int size;
  const REALTYPE value;  
  const REALTYPE init;

};



template<class REALTYPE>
class testMixSseLlo:public test<REALTYPE>{
public:
  testMixSseLlo():test<REALTYPE>(1/0.){
  }

};

template<>
class testMixSseLlo<double>:public test<double>{
 public:

  testMixSseLlo():test<double>(17){
  }

  std::string name(){
    return std::string("testMixSseLlo");
  }


  double compute(){
    const double a[]={1,2};
    const double b[]={3,4};
    const double c[]={5,6};
#if defined(TEST_SSE) && defined(__x86_64__)
    double res[2];
    __m128d bi,ci,ri;
    ri = _mm_loadu_pd(a);
    bi = _mm_loadu_pd(b);
    ri = _mm_add_sd(ri,bi);
    ci = _mm_loadu_pd(c);
    ri = _mm_add_pd(ri,ci);
    _mm_storeu_pd(res,ri);
    return res[0]+res[1];
#else
    return a[0]+a[1]+b[0]+c[0]+c[1];
#endif
  }
};


template<>
class testMixSseLlo<float>:public test<float>{
 public:

  testMixSseLlo():test<float>(57){
  }

  std::string name(){
    return std::string("testMixSseLlo");
  }


  float compute(){
    const float a[]={1,2,3,4};//Sum 10
    const float b[]={5,6,7,8};//Sum 5 because 6 7 8 will be ignored
    const float c[]={9,10,11,12};//Sum 42
#if defined(TEST_SSE) && defined(__x86_64__)
    float res[4];
    __m128 bi,ci,ri;
    ri = _mm_loadu_ps(a);
    bi = _mm_loadu_ps(b);
    ri=_mm_add_ss(ri,bi);
    ci = _mm_loadu_ps(c);
    ri=_mm_add_ps(ri,ci);
    _mm_storeu_ps(res,ri);
    return res[0]+res[1]+res[2]+res[3];
#else
    float res;
    return a[0]+a[1]+ a[2]+a[3]+ b[0] + c[0]+c[1]+ c[2]+c[3];
#endif
  }
};

template<class REALTYPE>
class testMixSseLlom:public test<REALTYPE>{
public:
  testMixSseLlom():test<REALTYPE>(1/0.){
  }

};

template<>
class testMixSseLlom<double>:public test<double>{
 public:

  testMixSseLlom():test<double>(-17){
  }

  std::string name(){
    return std::string("testMixSseLlo");
  }


  double compute(){
    const double a[]={-1,-2};
    const double b[]={-3,-4};
    const double c[]={-5,-6};
#if defined(TEST_SSE) && defined(__x86_64__)
    double res[2];
    __m128d bi,ci,ri;
    ri = _mm_loadu_pd(a);
    bi = _mm_loadu_pd(b);
    ri = _mm_add_sd(ri,bi);
    ci = _mm_loadu_pd(c);
    ri = _mm_add_pd(ri,ci);
    _mm_storeu_pd(res,ri);
    return res[0]+res[1];
#else
    return a[0]+a[1]+b[0]+c[0]+c[1];
#endif
  }
};



template<>
class testMixSseLlom<float>:public test<float>{
 public:

  testMixSseLlom():test<float>(-57){
  }

  std::string name(){
    return std::string("testMixSseLlom");
  }


  float compute(){
    const float a[]={-1,-2,-3,-4};//Sum -10
    const float b[]={-5,-6,-7,-8};//Sum -5 because -6 -7 -8 will be ignored
    const float c[]={-9,-10,-11,-12};//Sum -42

#if defined(TEST_SSE) && defined(__x86_64__)
    float res[4];
    __m128 bi,ci,ri;
    ri = _mm_loadu_ps(a);
    bi = _mm_loadu_ps(b);
    ri=_mm_add_ss(ri,bi);
    ci = _mm_loadu_ps(c);
    ri=_mm_add_ps(ri,ci);
    _mm_storeu_ps(res,ri);
    return res[0]+res[1]+res[2]+res[3];
#else
    float res;
    return a[0]+a[1]+ a[2]+a[3]+ b[0] + c[0]+c[1]+ c[2]+c[3];
#endif
  }
};




template<class REALTYPE,class REALTYPEREF>
class testCast:public test<REALTYPE,REALTYPEREF>{
  //test cast
public:
  testCast():test<REALTYPE,REALTYPEREF>(0.1){}

  std::string name(){
    return std::string("testCast");
  }

  REALTYPE compute(){
    volatile REALTYPEREF ref=0.1;
    return ((REALTYPE)ref);
  }
};

template<class REALTYPE,class REALTYPEREF>
class testCastm:public test<REALTYPE,REALTYPEREF>{
  //test cast -
public:
  testCastm():test<REALTYPE,REALTYPEREF>(-0.1){}

  std::string name(){
    return std::string("testCastm");
  }

  REALTYPE compute(){
    volatile REALTYPEREF ref=-0.1;
    return ((REALTYPE)ref);
  }
};



#if defined(TEST_SSE) && defined(__x86_64__)
  inline double mySqrt(const double& a){
    double d;
    __m128d ai,di;
    ai = _mm_load_sd(&a);
    di=_mm_sqrt_sd(ai,ai);
    d=_mm_cvtsd_f64(di);
    return d;
  }


  inline float mySqrt(const float& a){
    float d;
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    di=_mm_sqrt_ss(ai);
    d=_mm_cvtss_f32(di);

    return d;
  }
#else
template<class REALTYPE>
inline REALTYPE mySqrt(REALTYPE a);

template<>
inline double mySqrt<double>(double a){
  return __builtin_sqrt(a);
}
template<>
inline float mySqrt<float>(float a){
  return __builtin_sqrtf(a);
}
#endif

template<class REALTYPE>
class testDiffSqrt:public test<REALTYPE>{
 public:
  testDiffSqrt():test<REALTYPE>(0.),
    size(10000),
    value(0.1),
    init(0.){}

  std::string name(){
    return std::string("testDiffSqrt");
  }


  REALTYPE compute(){
    volatile REALTYPE acc=init;
    //    volatile REALTYPE acc2=-init;
    for(int i=0; i<size; i++){
      acc+= std::abs(mySqrt(value)- mySqrt(value)) ;
    }
    return acc;
  }

 private:
  const int size;
  volatile  REALTYPE value;
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
    testInc0d1 <RealType> t1; t1.run();
    testInc0d1m<RealType> t1m; t1m.run();
    testIncSquare0d1<RealType> t2; t2.run();
    testIncSquare0d1m<RealType> t2m; t2m.run();
    testIncDiv10<RealType> t3; t3.run();
    testIncDiv10m<RealType> t3m; t3m.run();
    testInvariantProdDiv<RealType> t4; t4.run();
    testInvariantProdDivm<RealType> t4m; t4m.run();
    testFma<RealType> t5; t5.run();
    testFmam<RealType> t5m; t5m.run();
    testMixSseLlo<RealType> t6; t6.run();
    testMixSseLlom<RealType> t6m; t6m.run();
    testCast<RealType,double> t7; t7.run();
    testCastm<RealType,double> t7m; t7m.run();
    testDiffSqrt<RealType> t8; t8.run();
  }

  {
    typedef float RealType;
    testInc0d1 <RealType> t1; t1.run();
    testInc0d1m<RealType> t1m; t1m.run();
    testIncSquare0d1<RealType> t2; t2.run();
    testIncSquare0d1m<RealType> t2m; t2m.run();
    testIncDiv10<RealType> t3; t3.run();
    testIncDiv10m<RealType> t3m; t3m.run();
    testInvariantProdDiv<RealType> t4; t4.run();
    testInvariantProdDivm<RealType> t4m; t4m.run();
    testFma<RealType> t5; t5.run();
    testFmam<RealType> t5m; t5m.run();
    testMixSseLlo<RealType> t6; t6.run();
    testMixSseLlom<RealType> t6m; t6m.run();
    testCast<RealType,double> t7; t7.run();
    testCastm<RealType,double> t7m; t7m.run();
    testDiffSqrt<RealType> t8; t8.run();
  }

  return EXIT_SUCCESS;
}
