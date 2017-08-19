
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

#include "interflop_verrou.h"
#include "vr_fpRepr.hxx"
#include "vr_fma.hxx"

//extern "C" {
#include "vr_rand.h"
//}


Vr_Rand vr_rand;

template <typename REAL>
void vr_checkCancellation (const REAL & a, const REAL & b, const REAL & r);

#include "vr_roundingOp.hxx"
#include "vr_op.hxx"


// * Global variables & parameters
int CHECK_C  = 0;
vr_RoundingMode DEFAULTROUNDINGMODE;
vr_RoundingMode ROUNDINGMODE;
unsigned int vr_seed;
void (*vr_cancellationHandler)(int)=NULL;
void (*vr_panicHandler)(const char*)=NULL;


void verrou_set_cancellation_handler(void (*cancellationHandler)(int)){
  vr_cancellationHandler=cancellationHandler;
}

void verrou_set_panic_handler(void (*panicHandler)(const char*)){
  vr_panicHandler=panicHandler;
}


void (*vr_debug_print_op)(int,const char*, const double*, const double*)=NULL;
void verrou_set_debug_print_op(void (*printOpHandler)(int nbArg,const char*name, const double* args,const double* res)){
  vr_debug_print_op=printOpHandler;
};


// * Operation implementation
const char*  verrou_rounding_mode_name (enum vr_RoundingMode mode) {
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
void vr_checkCancellation (const REAL & a, const REAL & b, const REAL & r) {
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
void IFV_FCTNAME(configure)(vr_RoundingMode mode,void* context) {
  DEFAULTROUNDINGMODE = mode;
  ROUNDINGMODE=mode;
}

void IFV_FCTNAME(finalyze)(void* context){
}

const char* IFV_FCTNAME(get_backend_name)() {
  return "verrou";
}

const char* IFV_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

void verrou_begin_instr(){
  ROUNDINGMODE=DEFAULTROUNDINGMODE;
}

void verrou_end_instr(){
  ROUNDINGMODE= VR_NEAREST;
}

void verrou_set_seed (unsigned int seed) {
  vr_seed = vr_rand_int (&vr_rand);
  vr_rand_setSeed (&vr_rand, seed);
}

void verrou_set_random_seed () {
  vr_rand_setSeed(&vr_rand, vr_seed);
}



void IFV_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <double>,double > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float>,float > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_doublex2) (doublex2 a, doublex2 b, doublex2* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <double>, doublex2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_floatx2) (floatx2 a, floatx2 b, floatx2* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float>, floatx2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_doublex4) (doublex4 a, doublex4 b, doublex4* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <double>,doublex4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_floatx4) (floatx4 a, floatx4 b, floatx4* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float>,floatx4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(add_floatx8) (floatx8 a, floatx8 b, floatx8* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float>,floatx8 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <double>,double > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <float>,float > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_doublex2) (doublex2 a, doublex2 b, doublex2* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <double>,doublex2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_floatx2) (floatx2 a, floatx2 b, floatx2* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <float>,floatx2> Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_doublex4) (doublex4 a, doublex4 b, doublex4* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <double>, doublex4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_floatx4) (floatx4 a, floatx4 b, floatx4* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <float>,floatx4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(sub_floatx8) (floatx8 a, floatx8 b, floatx8* res,void* context) {
  typedef OpWithSelectedRoundingMode<SubOp <float>,floatx8 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}


void IFV_FCTNAME(mul_double) (double a, double b, double* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <double> ,double> Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_float) (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float>,float > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_doublex2) (doublex2 a, doublex2 b, doublex2* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <double>,doublex2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_floatx2) (floatx2 a, floatx2 b, floatx2* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float> ,floatx2> Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_doublex4) (doublex4 a, doublex4 b, doublex4* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <double>, doublex4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_floatx4) (floatx4 a, floatx4 b, floatx4* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float>,floatx4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(mul_floatx8) (floatx8 a, floatx8 b, floatx8* res,void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float>,floatx8 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}


void IFV_FCTNAME(div_double) (double a, double b, double* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <double>,double > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_float) (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float>,float > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_doublex2) (doublex2 a, doublex2 b, doublex2* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <double>,doublex2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_floatx2) (floatx2 a, floatx2 b, floatx2* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float>,floatx2 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_doublex4) (doublex4 a, doublex4 b, doublex4* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <double>,doublex4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_floatx4) (floatx4 a, floatx4 b, floatx4* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float>,floatx4 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}

void IFV_FCTNAME(div_floatx8) (floatx8 a, floatx8 b, floatx8* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float>,floatx8 > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}



void IFV_FCTNAME(cast_double_to_float) (double a, float* res, void* context){
  typedef OpWithSelectedRoundingMode<CastOp<double,float>,float > Op;
  Op::apply(Op::PackArgs(a),res,context);
}

void IFV_FCTNAME(madd_double) (double a, double b, double c, double* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <double>,double > Op;
  Op::apply(Op::PackArgs(a,b,c), res,context);
}

void IFV_FCTNAME(madd_float) (float a, float b, float c, float* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <float>,float > Op;
  Op::apply(Op::PackArgs(a,b,c), res, context);
}




struct interflop_backend_interface_t IFV_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config;

  config.interflop_add_float = & IFV_FCTNAME(add_float);
  config.interflop_sub_float = & IFV_FCTNAME(sub_float);
  config.interflop_mul_float = & IFV_FCTNAME(mul_float);
  config.interflop_div_float = & IFV_FCTNAME(div_float);

  config.interflop_add_double = & IFV_FCTNAME(add_double);
  config.interflop_sub_double = & IFV_FCTNAME(sub_double);
  config.interflop_mul_double = & IFV_FCTNAME(mul_double);
  config.interflop_div_double = & IFV_FCTNAME(div_double);

  config.interflop_add_floatx2  = & IFV_FCTNAME(add_floatx2);
  config.interflop_sub_floatx2  = & IFV_FCTNAME(sub_floatx2);
  config.interflop_mul_floatx2  = & IFV_FCTNAME(mul_floatx2);
  config.interflop_div_floatx2  = & IFV_FCTNAME(div_floatx2);

  config.interflop_add_floatx4  = & IFV_FCTNAME(add_floatx4);
  config.interflop_sub_floatx4  = & IFV_FCTNAME(sub_floatx4);
  config.interflop_mul_floatx4  = & IFV_FCTNAME(mul_floatx4);
  config.interflop_div_floatx4  = & IFV_FCTNAME(div_floatx4);

  config.interflop_add_floatx8  = & IFV_FCTNAME(add_floatx8);
  config.interflop_sub_floatx8  = & IFV_FCTNAME(sub_floatx8);
  config.interflop_mul_floatx8  = & IFV_FCTNAME(mul_floatx8);
  config.interflop_div_floatx8  = & IFV_FCTNAME(div_floatx8);

  config.interflop_add_doublex2 = & IFV_FCTNAME(add_doublex2);
  config.interflop_sub_doublex2 = & IFV_FCTNAME(sub_doublex2);
  config.interflop_mul_doublex2 = & IFV_FCTNAME(mul_doublex2);
  config.interflop_div_doublex2 = & IFV_FCTNAME(div_doublex2);

  config.interflop_add_doublex4 = & IFV_FCTNAME(add_doublex4);
  config.interflop_sub_doublex4 = & IFV_FCTNAME(sub_doublex4);
  config.interflop_mul_doublex4 = & IFV_FCTNAME(mul_doublex4);
  config.interflop_div_doublex4 = & IFV_FCTNAME(div_doublex4);

  config.interflop_cast_double_to_float=& IFV_FCTNAME(cast_double_to_float);

  config.interflop_madd_float = & IFV_FCTNAME(madd_float);
  config.interflop_madd_double =& IFV_FCTNAME(madd_double);

  return config;
}
