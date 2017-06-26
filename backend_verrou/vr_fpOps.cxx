
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floating-point operations overloading.         ---*/
/*---                                                 vr_fpOps.cxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2016
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

//#include "pub_tool_libcfile.h"
#include "vr_fpRepr.hxx"
#include "vr_fpOps.h"
#include "vr_fma.hxx"

extern "C" {
#include "vr_rand.h"
}




template <typename REAL>
void checkCancellation (const REAL & a, const REAL & b, const REAL & r);

#include "vr_roundingOp.hxx"
#include "vr_op.hxx"


// * Global variables & parameters
int CHECK_C  = 0;
vr_RoundingMode DEFAULTROUNDINGMODE;
vr_RoundingMode ROUNDINGMODE;
unsigned int vr_seed;
void (*vr_cancellationHandler)(int);




void setCancellationHandler(void (*cancellationHandler)(int)){
  vr_cancellationHandler=cancellationHandler;
}


// * Operation implementation
inline const HChar*  roundingModeName (vr_RoundingMode mode) {
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
  case VR_FARTHEST:
    return "FARTHEST";
  }

  return "undefined";
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

  if (cancelled >= storedBits(a)) {
    vr_cancellationHandler(cancelled);
  }
}



// * C interface
void vr_fpOpsInit (vr_RoundingMode mode) {
  DEFAULTROUNDINGMODE = mode;
  ROUNDINGMODE=mode;

  if (ROUNDINGMODE == VR_RANDOM
      or ROUNDINGMODE == VR_AVERAGE) {
    VG_(umsg)("First seed : %u\n", vr_rand_getSeed (&vr_rand));
  }


  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (ROUNDINGMODE));


}

void vr_beginInstrumentation(){
  //  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (DEFAULTROUNDINGMODE));
  ROUNDINGMODE=DEFAULTROUNDINGMODE;
}

void vr_endInstrumentation(){
  //  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (VR_NEAREST));
  ROUNDINGMODE= VR_NEAREST;
}


void vr_fpOpsFini (void) {

}

void vr_fpOpsSeed (unsigned int seed) {
  vr_seed = vr_rand_int (&vr_rand);
  vr_rand_setSeed (&vr_rand, seed);
}

void vr_fpOpsRandom () {
  vr_rand_setSeed(&vr_rand, vr_seed);
}






void vr_AddDouble (double a, double b,double* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};

void vr_AddFloat (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};

void vr_SubDouble (double a, double b,double* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,-b),context);
};

void vr_SubFloat (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,-b),context);
};


void vr_MulDouble (double a, double b, double* res, void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};

void vr_MulFloat (float a, float b, float* res, void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};


void vr_DivDouble (double a, double b, double* res, void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};

void vr_DivFloat (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
};

void vr_MAddDouble (double a, double b, double c, double* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b,c),context);
};

void vr_MAddFloat (float a, float b, float c, float* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <float> > Op;
  *res= Op::apply(Op::PackArgs(a,b,c), context);
};




struct interflop_backend_interface_t interflop_BACKENDNAME_init(void ** context){
  struct interflop_backend_interface_t config;

  config.interflop_add_float = &vr_AddFloat;
  config.interflop_sub_float = &vr_SubFloat;;
  config.interflop_mul_float = &vr_MulFloat;
  config.interflop_div_float = &vr_DivFloat;

  config.interflop_add_double = &vr_AddDouble;
  config.interflop_sub_double = &vr_SubDouble;
  config.interflop_mul_double = &vr_MulDouble;
  config.interflop_div_double = &vr_DivDouble;

  config.interflop_add_floatx2 = NULL;
  config.interflop_sub_floatx2 = NULL;
  config.interflop_mul_floatx2 = NULL;
  config.interflop_div_floatx2 = NULL;

  config.interflop_add_floatx4 = NULL;
  config.interflop_sub_floatx4 = NULL;
  config.interflop_mul_floatx4 = NULL;
  config.interflop_div_floatx4 = NULL;

  config.interflop_add_floatx8 = NULL;
  config.interflop_sub_floatx8 = NULL;
  config.interflop_mul_floatx8 = NULL;
  config.interflop_div_floatx8 = NULL;

  config.interflop_add_doublex2 = NULL;
  config.interflop_sub_doublex2 = NULL;
  config.interflop_mul_doublex2 = NULL;
  config.interflop_div_doublex2 = NULL;

  config.interflop_add_doublex4 = NULL;
  config.interflop_sub_doublex4 = NULL;
  config.interflop_mul_doublex4 = NULL;
  config.interflop_div_doublex4 = NULL;


  config.interflop_madd_float = &vr_MAddFloat;
  config.interflop_madd_double =&vr_MAddDouble;

  return config;
}
