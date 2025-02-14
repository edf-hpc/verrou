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

#if defined(__aarch64__) && defined(TEST_FMA)
#include <arm_neon.h>
#endif

#if defined(TEST_SSE) || defined(TEST_AVX) || defined(TEST_FMA)
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
#if defined(__x86_64__)
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
#error "not yet implemented for this architecture"
#endif
#else
    d=a*b+c;
#endif
    return d;
  }


  inline float myFma(const float& a, const float& b, const float& c){
    float d;
#ifdef TEST_FMA
#if defined(__x86_64__)
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
#error "not yet implemented for this architecture"
#endif
#else
    d=a*b+c;
#endif
    return d;
  }


  inline double myFms(const double& a, const double& b, const double& c){
    double d;
#ifdef TEST_FMA
#if defined(__x86_64__)
    __m128d ai, bi,ci,di;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmsub_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);
#elif  defined(__aarch64__)
  const double nega=-a;
  const double negc=-c;
  const float64x1_t ai=vld1_f64(&nega);
  const float64x1_t bi=vld1_f64(&b);
  const float64x1_t ci=vld1_f64(&negc);
  const float64x1_t di=vfms_f64(ci,ai,bi);// warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  vst1_f64(&d, di);
#else
#error "not yet implemented for this architecture"
#endif
#else
    d=a*b-c;
#endif
    return d;
  }


  inline float myFms(const float& a, const float& b, const float& c){
    float d;
#ifdef TEST_FMA
#if defined(__x86_64__)
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    bi = _mm_load_ss(&b);
    ci = _mm_load_ss(&c);
    di=_mm_fmsub_ss(ai,bi,ci);
    d=_mm_cvtss_f32(di);
#elif  defined(__aarch64__)
  float av[2]={-a,0};
  float bv[2]={b,0};
  float cv[2]={-c,0};

  float32x2_t ap=vld1_f32(av);
  float32x2_t bp=vld1_f32(bv);
  float32x2_t cp=vld1_f32(cv);

  float32x2_t resp= vfms_f32(cp,ap,bp); // warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfms
  float res[2];
  vst1_f32(res, resp);
  d=res[0];
#else
#error "not yet implemented for this architecture"
#endif
#else
    d=a*b-c;
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
class testFms:public test<REALTYPE>{
 public:
  testFms():test<REALTYPE>(10000),
    size(1000000),
    value(0.1),
    init(0.){}

  std::string name(){
    return std::string("testFms");
  }


  REALTYPE compute(){
    REALTYPE acc=init;
    volatile REALTYPE mone=-1.;
    for(int i=0; i<size; i++){
      acc=myFms(value,value,mone*acc);
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
class testMixVectorLlo;

typedef enum  {seq,sse,avx} VectorType_t;

template<>
class testMixVectorLlo<double>:public test<double>{
public:
  double a[4],b[4],c[4];
#if defined(TEST_SSE) && defined(__x86_64__)
  __m128d ai_sse,bi_sse,ci_sse;
#endif
#if defined(TEST_AVX) && defined(__x86_64__)
  __m256d ai_avx,bi_avx,ci_avx;
#endif

  VectorType_t vectorType_;

  testMixVectorLlo(double expectedResult, VectorType_t vectorType):
    test<double>(expectedResult),
    vectorType_(vectorType){
    double aTmp[]= {1.1  , 2.1  , 3.1  , 4.1 } ;
    double bTmp[]= {5.01 , 6.01 , 7.01 , 8.01} ;
    double cTmp[]= {9    , 10   , 11   , 12  } ;  //sum42
    memcpy(a, aTmp, 4*sizeof(double));
    memcpy(b, bTmp, 4*sizeof(double));
    memcpy(c, cTmp, 4*sizeof(double));
}

  void minusA(){
    for(int i=0; i<4;i++) a[i]*=-1;
  }
  void minusB(){
    for(int i=0; i<4;i++) b[i]*=-1;
  }
  void minusC(){
    for(int i=0; i<4;i++) c[i]*=-1;
  }

  void load(){
    switch(vectorType_){
    case seq: return;
    case sse: loadSSE(); return;
    case avx: loadAVX(); return;
    default: std::cerr << "invalid type"<<std::endl;
    }
  }
  void store(){
    switch(vectorType_){
    case seq: return;
    case sse: storeSSE(); return;
    case avx: storeAVX(); return;
    default: std::cerr << "invalid type"<<std::endl;
    }
  }

  void loadSSE(){
#if  defined(TEST_SSE) && defined(__x86_64__)
    ai_sse = _mm_loadu_pd(a);
    bi_sse = _mm_loadu_pd(b);
    ci_sse = _mm_loadu_pd(c);
#endif
  }
  void storeSSE(){
#if defined(TEST_SSE) && defined(__x86_64__)
    _mm_storeu_pd(c,ci_sse);
#endif
  }

  void loadAVX(){
#if defined(TEST_AVX)  && defined(__x86_64__)
    ai_avx = _mm256_loadu_pd(&(a[0]));
    bi_avx = _mm256_loadu_pd(&(b[0]));
    ci_avx = _mm256_loadu_pd(&(c[0]));
#endif
  }
  void storeAVX(){
#if defined(TEST_AVX)  && defined(__x86_64__)
    _mm256_storeu_pd(c,ci_avx);
#endif
  }

  virtual void applyOPs()=0;

  double compute(){
    load();
    applyOPs();
    store();
    return accResult();
  }

  double accResult(){
    //    std::cout << c[0] <<"\t"<< c[1] <<"\t"<<  c[2]<<"\t"<< c[3]<<std::endl;
    return c[0]+c[1]+c[2]+c[3];
  }
};

template<>
class testMixVectorLlo<float>:public test<float>{
public:

  float a[8], b[8], c[8];

#if defined(TEST_SSE)  && defined(__x86_64__)
  __m128 ai_sse,bi_sse,ci_sse;
#endif
#if defined(TEST_AVX)  && defined(__x86_64__)
  __m256 ai_avx,bi_avx,ci_avx;
#endif

  VectorType_t vectorType_;

  testMixVectorLlo(float expectedResult, VectorType_t vectorType):
    test<float>(expectedResult),
    vectorType_(vectorType){

    float aTmp[]={1.1 , 2.1 , 3.1 , 4.1 , 5.1 , 6.1  , 7.1  , 8.1   };
    float bTmp[]={5.01, 6.01, 7.01, 8.01, 9.01, 10.01, 11.01, 12.01 };
    float cTmp[]={9   , 10  , 11  , 12  , 13  , 14   , 15   ,16     }; //sum 100
    memcpy(a, aTmp, 8*sizeof(float));
    memcpy(b, bTmp, 8*sizeof(float));
    memcpy(c, cTmp, 8*sizeof(float));
  }
  void minusA(){
    for(int i=0; i<8;i++) a[i]*=-1;
  }
  void minusB(){
    for(int i=0; i<8;i++) b[i]*=-1;
  }
  void minusC(){
    for(int i=0; i<8;i++) c[i]*=-1;
  }

  void load(){
    switch(vectorType_){
    case seq: return;
    case sse: loadSSE(); return;
    case avx: loadAVX(); return;
    default: std::cerr << "invalid type"<<std::endl;
    }
  }
  void store(){
    switch(vectorType_){
    case seq: return;
    case sse: storeSSE(); return;
    case avx: storeAVX(); return;
    default: std::cerr << "invalid type"<<std::endl;
    }
  }

  void loadSSE(){
#if defined(TEST_SSE)  && defined(__x86_64__)
    ai_sse = _mm_loadu_ps(a);
    bi_sse = _mm_loadu_ps(b);
    ci_sse = _mm_loadu_ps(c);
#endif
  }
  void storeSSE(){
#if defined(TEST_SSE)  && defined(__x86_64__)
    _mm_storeu_ps(c,ci_sse);
#endif
  }

  void loadAVX(){
#if defined(TEST_AVX)  && defined(__x86_64__)
    ai_avx = _mm256_loadu_ps(a);
    bi_avx = _mm256_loadu_ps(b);
    ci_avx = _mm256_loadu_ps(c);
#endif
  }
  void storeAVX(){
#if defined(TEST_AVX)  && defined(__x86_64__)
    _mm256_storeu_ps(c,ci_avx);
#endif
  }

  float accResult(){
    return c[0]+c[1]+c[2]+c[3]+c[4]+c[5]+c[6]+c[7];
  }

  virtual void applyOPs()=0;
  float compute(){
    load();
    applyOPs();
    store();
    return accResult();
  }

};




template<class REALTYPE>
class testMixSseLlo;

template<>
class testMixSseLlo<double>:public testMixVectorLlo<double>{
 public:

  testMixSseLlo():testMixVectorLlo<double>(31.21,sse){//42-9+6.11 -10 +2.1
  }

  std::string name(){
    return std::string("testMixSseLlo");
  }

  void applyOPs(){
#if defined(TEST_SSE) && defined(__x86_64__)
    ci_sse = _mm_add_sd(ai_sse,bi_sse);
#else
    c[0]=a[0]+b[0];
    c[1]=a[1];
#endif
  }
};

template<>
class testMixSseLlo<float>:public testMixVectorLlo<float>{
 public:

  testMixSseLlo():testMixVectorLlo<float>(73.41, sse){//100-9+6.11 -10 +2.1 -11 +3.1 -12+4.1
  }

  std::string name(){
    return std::string("testMixSseLlo");
  }

  void applyOPs(){
#if defined(TEST_SSE) && defined(__x86_64__)
    ci_sse = _mm_add_ss(ai_sse,bi_sse);
#else
    c[0]=a[0]+b[0];
#endif
  }
};



template<class REALTYPE>
class testMixSseLlom:public testMixSseLlo<REALTYPE>{
public:
  testMixSseLlom():testMixSseLlo<REALTYPE>(){
    testMixSseLlo<REALTYPE>::minusA();
    testMixSseLlo<REALTYPE>::minusB();
    testMixSseLlo<REALTYPE>::minusC();
    testMixSseLlo<REALTYPE>::expectedResult *=-1;
  }
  std::string name(){
    return std::string("testMixSseLlom");
  }
};



template<class REALTYPE>
class testMixAvxLlo;

template<>
class testMixAvxLlo<double>:public testMixVectorLlo<double>{
 public:

  testMixAvxLlo():testMixVectorLlo<double>(12.12,seq){
    // seq because llo avx intrinsics do not exist
  }

  std::string name(){
    return std::string("testMixAvxLlo");
  }

  void applyOPs(){
#if defined(TEST_AVX) && defined(__x86_64__)
    __asm__("vmovupd %0,%%ymm0\n"
	    "vmovupd %1,%%ymm1\n"
	    "vmovupd %2,%%ymm2\n"
	    "vaddsd  %%xmm1,%%xmm2,%%xmm0;"
	    "vmovupd %%ymm0, %0\n"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]=a[0]+b[0];
    c[1]=b[1];
    c[2]=0.;
    c[3]=0.;
#endif
  }
};

template<>
class testMixAvxLlo<float>:public testMixVectorLlo<float>{
 public:

  testMixAvxLlo():testMixVectorLlo<float>(27.14,seq){
    // seq because llo avx intrinsics do not exist
  }

  std::string name(){
    return std::string("testMixAvxLlo");
  }

  void applyOPs(){
#if defined(TEST_AVX) && defined(__x86_64__)
    __asm__("vmovups %0,%%ymm0\n"
	    "vmovups %1,%%ymm1\n"
	    "vmovups %2,%%ymm2\n"
	    "vaddss  %%xmm1,%%xmm2,%%xmm0;"
	    "vmovups %%ymm0, %0\n"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]=a[0]+b[0];
    c[1]=b[1];
    c[2]=b[2];
    c[3]=b[3];
    c[4]=0.0;
    c[5]=0.0;
    c[6]=0.0;
    c[7]=0.0;
#endif
  }
};


template<class REALTYPE>
class testMixAvxLlom:public testMixAvxLlo<REALTYPE>{
public:
  testMixAvxLlom():testMixAvxLlo<REALTYPE>(){
    testMixAvxLlo<REALTYPE>::minusA();
    testMixAvxLlo<REALTYPE>::minusB();
    testMixAvxLlo<REALTYPE>::minusC();
    testMixAvxLlo<REALTYPE>::expectedResult *=-1;
  }
  std::string name(){
    return std::string("testMixAvxLlom");
  }
};


template<class REALTYPE>
class testFmaMixAvxLlo;

template<>
class testFmaMixAvxLlo<double>:public testMixVectorLlo<double>{
 public:

  testFmaMixAvxLlo():testMixVectorLlo<double>(16.611,seq){
    // seq because llo avx intrinsics do not exist
  }

  std::string name(){
    return std::string("testFmaMixAvxLlo");
  }

  void applyOPs(){
#if defined(TEST_AVX) && defined(TEST_FMA) && defined(__x86_64__)
    __asm__("vmovupd %0,%%ymm0\n"
	    "vmovupd %1,%%ymm1\n"
	    "vmovupd %2,%%ymm2\n"
	    "vfmadd213sd  %%xmm0,%%xmm2,%%xmm1;"
	    "vmovupd %%ymm1, %0\n"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]+=a[0]*b[0];
    c[1]=a[1];
    c[2]=0.;
    c[3]=0.;
#endif
  }
};



template<>
class testFmaMixAvxLlo<float>:public testMixVectorLlo<float>{
 public:

  testFmaMixAvxLlo():testMixVectorLlo<float>(23.811, seq){//100-9+6.11 -10 +2.1 -11 +3.1 -12+4.1 -13 -14 -15-16
  }
  //seq because asm is required

  std::string name(){
    return std::string("testFmaMixAvxLlo");
  }

  void applyOPs(){
#if defined(TEST_AVX) && defined(TEST_FMA) && defined(__x86_64__)
	__asm__("vmovups %0,%%ymm0\n"
	    "vmovups %1,%%ymm1\n"
	    "vmovups %2,%%ymm2\n"
	    "vfmadd213ss  %%xmm0,%%xmm2,%%xmm1\n"
	    "vmovups %%ymm1, %0;"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]+=a[0]*b[0];
    c[1]=a[1];
    c[2]=a[2];
    c[3]=a[3];
    c[4]=0.;
    c[5]=0.;
    c[6]=0.;
    c[7]=0.;
#endif
  }
};

template<class REALTYPE>
class testFmaMixAvxLlom:public testFmaMixAvxLlo<REALTYPE>{
public:
  testFmaMixAvxLlom():testFmaMixAvxLlo<REALTYPE>(){
    testFmaMixAvxLlo<REALTYPE>::minusA();
    testFmaMixAvxLlo<REALTYPE>::minusC();
    testFmaMixAvxLlo<REALTYPE>::expectedResult *=-1;
  }
  std::string name(){
    return std::string("testFmaMixAvxLlom");
  }
};

template<class REALTYPE>
class testFmaMixSseLlo;

template<>
class testFmaMixSseLlo<double>:public testMixVectorLlo<double>{
 public:

  testFmaMixSseLlo():testMixVectorLlo<double>(39.611,seq){
    // seq because llo avx intrinsics do not exist
  }

  std::string name(){
    return std::string("testFmaMixSseLlo");
  }

  void applyOPs(){
#if defined(TEST_SSE) && defined(TEST_FMA) && defined(__x86_64__)
    __asm__("vmovupd %0,%%xmm0\n"
	    "vmovupd %1,%%xmm1\n"
	    "vmovupd %2,%%xmm2\n"
	    "vfmadd213sd  %%xmm0,%%xmm2,%%xmm1;"
	    "vmovupd %%xmm1, %0\n"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]+=a[0]*b[0];
    c[1]=a[1];
#endif
  }
};



template<>
class testFmaMixSseLlo<float>:public testMixVectorLlo<float>{
 public:

  testFmaMixSseLlo():testMixVectorLlo<float>(81.811, seq){
  }
  //seq because asm is required

  std::string name(){
    return std::string("testFmaMixSseLlo");
  }

  void applyOPs(){
#if defined(TEST_SSE) && defined(TEST_FMA) && defined(__x86_64__)
	__asm__("vmovups %0,%%xmm0\n"
	    "vmovups %1,%%xmm1\n"
	    "vmovups %2,%%xmm2\n"
	    "vfmadd213ss  %%xmm0,%%xmm2,%%xmm1\n"
	    "vmovups %%xmm1, %0;"
            : "=m" (c) : "m" (a), "m" (b));
#else
    c[0]+=a[0]*b[0];
    c[1]=a[1];
    c[2]=a[2];
    c[3]=a[3];
#endif
  }
};

template<class REALTYPE>
class testFmaMixSseLlom:public testFmaMixSseLlo<REALTYPE>{
public:
  testFmaMixSseLlom():testFmaMixSseLlo<REALTYPE>(){
    testFmaMixSseLlo<REALTYPE>::minusA();
    testFmaMixSseLlo<REALTYPE>::minusC();
    testFmaMixSseLlo<REALTYPE>::expectedResult *=-1;
  }
  std::string name(){
    return std::string("testFmaMixSseLlom");
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
    testFms<RealType> t5s; t5s.run();
    testMixSseLlo<RealType> t6; t6.run();
    testMixSseLlom<RealType> t6m; t6m.run();
    testCast<RealType,double> t7; t7.run();
    testCastm<RealType,double> t7m; t7m.run();
    testDiffSqrt<RealType> t8; t8.run();

    testFmaMixSseLlo<RealType> t9; t9.run();
    testFmaMixSseLlom<RealType> t9m; t9m.run();
    testFmaMixAvxLlo<RealType> t10; t10.run();
    testFmaMixAvxLlom<RealType> t10m; t10m.run();

    testMixAvxLlo<RealType> t11; t11.run();
    testMixAvxLlom<RealType> t11m; t11m.run();
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
    testFms<RealType> t5s; t5s.run();
    testMixSseLlo<RealType> t6; t6.run();
    testMixSseLlom<RealType> t6m; t6m.run();
    testCast<RealType,double> t7; t7.run();
    testCastm<RealType,double> t7m; t7m.run();
    testDiffSqrt<RealType> t8; t8.run();

    testFmaMixSseLlo<RealType> t9; t9.run();
    testFmaMixSseLlom<RealType> t9m; t9m.run();
    testFmaMixAvxLlo<RealType> t10; t10.run();
    testFmaMixAvxLlom<RealType> t10m; t10m.run();

    testMixAvxLlo<RealType> t11; t11.run();
    testMixAvxLlom<RealType> t11m; t11m.run();
  }

  return EXIT_SUCCESS;
}
