#include <cstdlib>
#include <ctime>
//#include <iostream>
#include <cmath>
#include "vr_fpOps.h"
#include "vr_DekkerOps.h"

#include "pub_tool_vki.h"

extern "C" {
#include <stdio.h>
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcassert.h"

}

#include "vr_fpRepr.hxx"

// * Global variables & parameters

vr_RoundingMode DEFAULTROUNDINGMODE;
vr_RoundingMode ROUNDINGMODE;
//Bool vr_instrument_state;

const int CHECK_IP = 0;
const int CHECK_C  = 0;

int vr_outFile;
unsigned int vr_seed;


// * Operation implementation

inline char const*const roundingModeName (vr_RoundingMode mode) {
  switch (mode) {
  case VR_NEAREST:
    return "NEAREST";
  case VR_UPWARD:
    return "UPWARD";
  case VR_DOWNWARD:
    return "DOWNWARD";
  case VR_ZERO:
    return "TOWARD_ZERO";
  case VR_RANDOM:
    return "RANDOM";
  case VR_AVERAGE:
    return "AVERAGE";
  }

  return "undefined";
}

template <typename REAL>
inline vr_RoundingMode getMode (vr_RoundingMode mode, const REAL & err, const REAL & ulp) {
  switch(mode) {
  case VR_RANDOM:
    return (vr_RoundingMode)(rand()%VR_RANDOM);
  case VR_AVERAGE: {
    int s = err>=0 ? 1 : -1;

    if (rand() * ulp < RAND_MAX * s * err) {
      // Probability: abs(err)/ulp
      if (s>0) {
        return VR_UPWARD;
      } else {
        return VR_DOWNWARD;
      }
    } else {
      return VR_NEAREST;
    }
  }
  default:
    return mode;
  }
}

// ** Addition

template <typename REAL>
void checkInsufficientPrecision (const REAL & a, const REAL & b) {
  if (CHECK_IP == 0)
    return;

  const int ea = exponentField (a);
  const int eb = exponentField (b);

  const int emax = ea>eb ? ea : eb;
  const int emin = ea<eb ? ea : eb;

  const int n = storedBits(a);
  const int dp = 1+emax-emin-n;

  char s[256];
  if (dp>0) {
    const int l = VG_(sprintf)(s, "IP %d\n", dp);
    VG_(write)(vr_outFile, s, l);
  }
}

template <typename REAL>
void checkCancellation (const REAL & a, const REAL & b, const REAL & r) {
  if (CHECK_C == 0)
    return;

  const int ea = exponentField (a);
  const int eb = exponentField (b);
  const int er = exponentField (r);

  const int emax = ea>eb ? ea : eb;
  const int cancelled = emax - er;

  char s[256];
  if (cancelled >= storedBits(a)) {
    const int l = VG_(sprintf)(s, "C  %d\n", cancelled);
    VG_(write)(vr_outFile, s, l);
  }
}

template<class REAL,  vr_RoundingMode ROUND>
class ValErr {
public:
  REAL value;
  REAL error;
  

  inline
  void changeValue(){
    if(error>0){
      value=nextAfter<REAL>(value);
      return;
    }
    if(error<0){ 
      value=nextPrev<REAL>(value);
      return;
    }
  }

  inline
  void changeValue(REAL ulp){
    if(error>0){
      value+=ulp;
      return;
    }
    if(error<0){ 
      value-=ulp;
      return;
    }
  }


  inline void applyRounding(){
    

    //    const vr_RoundingMode mode = getMode (ROUND, error, u);
    if(ROUND==VR_NEAREST) return;


    
    if(ROUND==VR_RANDOM){
      bool doNoChange=(rand()%2==0);
      if(doNoChange){
	return;
      }
      if(error!=0.){	
	changeValue();return;
      }
    }

    if(ROUND==VR_AVERAGE){
      const int s = error>=0 ? 1 : -1;
      const REAL u =ulp(value);      
      bool doChange= (rand() * u < RAND_MAX * s * error);
      // Probability: abs(err)/ulp
      if(doChange){
	changeValue(u);return;
      }


    }

    if (error > 0
        && (ROUND == VR_UPWARD
            || (ROUND == VR_ZERO && value < 0))){
      value =nextAfter<REAL>(value);
      return;
    }
    if (error < 0
        && (ROUND == VR_DOWNWARD
            || (ROUND == VR_ZERO && value > 0))){
      value =nextPrev<REAL>(value);
      return;
    }



  }
    
};


template <vr_RoundingMode ROUND>
class Sum {
public:

  template<typename REAL>
  static REAL apply (const REAL & a, const REAL & b) {
    /*    if (a == 0)
      return b;
    if (b == 0)
    return a;*/

    //    ValErr ve = (std::abs(a) < std::abs(b)) ?
    //      priest_ (b, a):
    //      priest_ (a, b);
    ValErr<REAL,ROUND> ve;
    DekkerOp<REAL>::sum12(a,b,ve.value,ve.error);
    //DekkerOp<REAL>::priest(a,b,ve.value,ve.error);
    ve.applyRounding();
    //    ve.checkNearest("Add",a+b);
    checkInsufficientPrecision (a, b);
    checkCancellation (a, b, ve.value);

    return ve.value;
  }

};



template <vr_RoundingMode ROUND>
class Mul {
public:
  template<typename REAL>
  static REAL apply (const REAL & a, const REAL & b) {

    ValErr<REAL,ROUND> ve;
    DekkerOp<REAL>::twoProd(a,b,ve.value,ve.error);
    //    ve.applyRoundingProd();
    ve.applyRounding();
    //ve.checkNearest("Mult",a*b);
    return ve.value;
  }

};


template <vr_RoundingMode ROUND>
class Div {
public:
  template<typename REAL>
  static REAL apply (const REAL & a, const REAL & b) {
    if (a == 0)
      return 0;

    ValErr<REAL,ROUND> ve;
    DekkerOp<REAL>::div12(a,b,ve.value,ve.error);
    ve.applyRounding();
    //    ve.checkNearest("Div",a/b);
    return ve.value;
  }

};





// * C interface

void vr_fpOpsInit (vr_RoundingMode mode) {
  DEFAULTROUNDINGMODE = mode;
  ROUNDINGMODE=mode;
  
  if (ROUNDINGMODE >= VR_RANDOM) {
    srand (time (NULL));
  }

  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (ROUNDINGMODE));

  vr_outFile = VG_(fd_open)("vr.log",
                            VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
                            VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
}

void vr_beginInstrumentation(){
  ROUNDINGMODE=DEFAULTROUNDINGMODE;
}

void vr_endInstrumentation(){
  //  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (VR_NEAREST));
  ROUNDINGMODE= VR_NEAREST;
}


void vr_fpOpsFini (void) {
  VG_(close)(vr_outFile);
}

void vr_fpOpsSeed (unsigned int seed) {
  vr_seed = rand();
  srand (seed);
}

void vr_fpOpsRandom () {
  srand (vr_seed);
}


template<template<vr_RoundingMode> class OP >
class OpWithSelectedRoundingMode{
public:
template<class REALTYPE>
static REALTYPE apply(REALTYPE a, REALTYPE b){
  switch (ROUNDINGMODE) {
  case VR_NEAREST:
    return OP<VR_NEAREST>::apply (a, b);
  case VR_UPWARD:
    return OP<VR_UPWARD>::apply (a, b);
  case VR_DOWNWARD:
    return OP<VR_DOWNWARD>::apply (a, b);
  case VR_ZERO:
    return OP<VR_ZERO>::apply (a, b);
  case VR_RANDOM:
    return OP<VR_RANDOM>::apply (a, b);
  case VR_AVERAGE:
    return OP<VR_AVERAGE>::apply (a, b);
  }
  return 0;
}
};

double vr_AddDouble (double a, double b) {
  return OpWithSelectedRoundingMode<Sum>::apply(a,b);
}

float vr_AddFloat (float a, float b) {
  return OpWithSelectedRoundingMode<Sum>::apply(a,b);
}


double vr_MulDouble (double a, double b) {
  return OpWithSelectedRoundingMode<Mul>::apply(a,b);
}

float vr_MulFloat (float a, float b) {
  return OpWithSelectedRoundingMode<Mul>::apply(a,b);
}


double vr_DivDouble (double a, double b) {
  return OpWithSelectedRoundingMode<Div>::apply(a,b);
}

float vr_DivFloat (float a, float b) {
  return OpWithSelectedRoundingMode<Div>::apply(a,b);
}

int vr_signDouble(double x){
  return sign(x);
}
void vr_ppDouble(double x){
  //  VG_(umsg)("Problems sign : %d %f\n", *arg1,*arg2);
  FPType<double>::Repr::pp(x);

}
