
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

#include "pub_tool_libcfile.h"
#include "vr_fpRepr.hxx"
#include "vr_fpOps.h"
#include "vr_fma.hxx"

extern "C" {
#include "vr_rand.h"
}


// Forward declarations
template <typename REAL>
void checkInsufficientPrecision (const REAL & a, const REAL & b);
template <typename REAL>
void checkCancellation (const REAL & a, const REAL & b, const REAL & r);

#include "vr_roundingOp.hxx"
#include "vr_op.hxx"


// * Global variables & parameters
vr_RoundingMode DEFAULTROUNDINGMODE;
vr_RoundingMode ROUNDINGMODE;
//Bool vr_instrument_state;

const int CHECK_IP = 0;
const int CHECK_C  = 0;

VgFile * vr_outFile;

unsigned int vr_seed;


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

  if (dp>0) {
    VG_(fprintf)(vr_outFile, "IP %d\n", dp);
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

  if (cancelled >= storedBits(a)) {
    VG_(fprintf)(vr_outFile, "C  %d\n", cancelled);
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

  if (CHECK_IP != 0) {
    vr_outFile = VG_(fopen)("vr.log",
			      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
			      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
  }
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
  if (CHECK_IP != 0) {
    VG_(fclose)(vr_outFile);
  }
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
}

void vr_AddFloat (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<AddOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
}


void vr_MulDouble (double a, double b, double* res, void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
}

void vr_MulFloat (float a, float b, float* res, void* context) {
  typedef OpWithSelectedRoundingMode<MulOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
}


void vr_DivDouble (double a, double b, double* res, void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
}

void vr_DivFloat (float a, float b, float* res,void* context) {
  typedef OpWithSelectedRoundingMode<DivOp <float> > Op;
  *res=Op::apply(Op::PackArgs(a,b),context);
}

void vr_MAddDouble (double a, double b, double c, double* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <double> > Op;
  *res=Op::apply(Op::PackArgs(a,b,c),context);
}

void vr_MAddFloat (float a, float b, float c, float* res, void* context){
  typedef OpWithSelectedRoundingMode<MAddOp <float> > Op;
  *res= Op::apply(Op::PackArgs(a,b,c), context);
}
