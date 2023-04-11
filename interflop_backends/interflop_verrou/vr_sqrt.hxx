
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains low-level code calling FMA instructions.  ---*/
/*---                                                   vr_fma.hxx ---*/
/*--------------------------------------------------------------------*/

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


#pragma once

#ifdef    USE_VERROU_SQRT
#include  <immintrin.h>
#endif
//#include  <fmaintrin.h>

template<class REALTYPE>
inline REALTYPE vr_sqrt(const REALTYPE&){
  return 0./ 0.; //nan to be sur not used
}

template<>
inline double vr_sqrt<double>(const double& a){
#ifdef USE_VERROU_SQRT
  double d;
  __m128d ai,resi;
  ai = _mm_load_sd(&a);
  resi=_mm_sqrt_sd(ai,ai);
  d=_mm_cvtsd_f64(resi);
  return d;
#else
  return 0./ 0.; //nan to be sur not used
#endif
}


template<>
inline float vr_sqrt<float>(const float& a){
#ifdef USE_VERROU_SQRT
  float d;
  __m128 ai,resi;
  ai = _mm_load_ss(&a);
  resi=_mm_sqrt_ss(ai);
  d=_mm_cvtss_f32(resi);
  return d;
#else
  return 0./ 0.; //nan to be sur not used
#endif
  
}

