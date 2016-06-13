
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

#include <cstdlib>
#include <ctime>
//#include <iostream>
//#include <cmath>
#include "math.h"
#include "vr_fpOps.h"
#include "vr_fma.h"

#include "pub_tool_vki.h"

extern "C" {
#include <stdio.h>
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcassert.h"

}

#include "vr_fpRepr.hxx"
#include "vr_rand.hxx"

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

int vr_outFile;

unsigned int vr_seed;
vrRand vr_rand;


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



// * C interface
void vr_fpOpsInit (vr_RoundingMode mode, unsigned int pid) {
  DEFAULTROUNDINGMODE = mode;
  ROUNDINGMODE=mode;

  if (ROUNDINGMODE == VR_RANDOM
      or ROUNDINGMODE == VR_AVERAGE) {
    vr_rand.setTimeSeed(pid);
  }


  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (ROUNDINGMODE));

  if (CHECK_IP != 0) {
    vr_outFile = VG_(fd_open)("vr.log",
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
  VG_(close)(vr_outFile);
}

void vr_fpOpsSeed (unsigned int seed) {
  vr_seed = vr_rand.getRandomInt();
  vr_rand.setSeed(seed);
}

void vr_fpOpsRandom () {
  vr_rand.setSeed(vr_seed);
}






double vr_AddDouble (double a, double b) {
  typedef OpWithSelectedRoundingMode<AddOp <double> > Op;
  return Op::apply(Op::PackArgs(a,b));
}

float vr_AddFloat (float a, float b) {
  typedef OpWithSelectedRoundingMode<AddOp <float> > Op;
  return Op::apply(Op::PackArgs(a,b));
}


double vr_MulDouble (double a, double b) {
  typedef OpWithSelectedRoundingMode<MulOp <double> > Op;
  return Op::apply(Op::PackArgs(a,b));
}

float vr_MulFloat (float a, float b) {
  typedef OpWithSelectedRoundingMode<MulOp <float> > Op;
  return Op::apply(Op::PackArgs(a,b));
}


double vr_DivDouble (double a, double b) {
  typedef OpWithSelectedRoundingMode<DivOp <double> > Op;
  return Op::apply(Op::PackArgs(a,b));
}

float vr_DivFloat (float a, float b) {
  typedef OpWithSelectedRoundingMode<DivOp <float> > Op;
  return Op::apply(Op::PackArgs(a,b));
}

double vr_MAddDouble (double a, double b, double c){
  typedef OpWithSelectedRoundingMode<MAddOp <double> > Op;
  return Op::apply(Op::PackArgs(a,b,c));
}

float vr_MAddFloat (float a, float b, float c){
  typedef OpWithSelectedRoundingMode<MAddOp <float> > Op;
  return Op::apply(Op::PackArgs(a,b,c));
}
