/*****************************************************************************\
 *                                                                           *\
 *  This file is part of the Verificarlo project,                            *\
 *  under the Apache License v2.0 with LLVM Exceptions.                      *\
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                 *\
 *  See https://llvm.org/LICENSE.txt for license information.                *\
 *                                                                           *\
 *  Copyright (c) 2024                                                       *\
 *     Verificarlo Contributors                                              *\
 *                                                                           *\
 ****************************************************************************/

#include "interflop_fma.h"
#ifndef HAS_QUADMATH
#include "pfp128.h"
#endif

float interflop_fma_binary32(float a, float b, float c) {
#ifdef __has_builtin
  return __builtin_fmaf(a, b, c);
#else
  float d;
#if defined(__x86_64__)
  __m128 ai, bi,ci,di;
  ai = _mm_load_ss(&a);
  bi = _mm_load_ss(&b);
  ci = _mm_load_ss(&c);
  di=_mm_fmadd_ss(ai,bi,ci);
  d=_mm_cvtss_f32(di);
#elif defined(____aarch64__)
  __asm__(
	  "fmadd %s0, %s1, %s2, %s3":
	  "=x"(d): "x"(a),"x"(b),"x"(c)
	  );
  #else
#error("arch missing")
#endif
  return d;
#endif
}

double interflop_fma_binary64(double a, double b, double c) {
#ifdef __has_builtin
  return __builtin_fma(a, b, c);
#else
  double d;
#if defined(__x86_64__)
  __m128d ai, bi,ci,di;
  ai = _mm_load_sd(&a);
  bi = _mm_load_sd(&b);
  ci = _mm_load_sd(&c);
  di=_mm_fmadd_sd(ai,bi,ci);
  d=_mm_cvtsd_f64(di);
#elif defined(____aarch64__)
  const float64x1_t ai=vld1_f64(&a);
  const float64x1_t bi=vld1_f64(&b);
  const float64x1_t ci=vld1_f64(&c);

  const float64x1_t di=vfma_f64(ci,ai,bi);// warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  vst1_f64(&d, di);
#else
#error("arch missing")
#endif
  return d;
#endif
}

#ifndef IGNORE_FMA128
_Float128 interflop_fma_binary128(_Float128 a, _Float128 b, _Float128 c) {
#ifdef HAS_QUADMATH
  return fmaq(a, b, c);
#else
  return fmaFP128(a, b, c);
#endif
}
#endif
