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

#include "interflop_sqrt.h"

#if defined(VGA_amd64) || defined(__x86_64__)
#include  <immintrin.h>
#endif

#if defined(VGA_arm64) || defined(__aarch64__)
#include "arm_neon.h"
#endif


float interflop_sqrt_binary32(float a) {
//#ifdef __has_builtin
//  return __builtin_sqrtf(a);
//#else

#if defined(__x86_64__)
  float d;
  __m128 ai, di;
  ai = _mm_load_ss(&a);
  di=_mm_sqrt_ss(ai);
  d=_mm_cvtss_f32(di);
  return d;
#elif defined(__aarch64__)
  float av[2]={a,0};
  float32x2_t ap=vld1_f32(av);
  float32x2_t resp= vsqrt_f32(ap);
  float res[2];
  vst1_f32(res, resp);
  return res[0];
  #else
#error("arch missing")
#endif
//#endif
}

double interflop_sqrt_binary64(double a) {
//#ifdef __has_builtin
//  return __builtin_sqrt(a);
//#else
  double d;
#if defined(__x86_64__)
  __m128d ai,di;
  ai = _mm_load_sd(&a);
  di=_mm_sqrt_sd(ai,ai);
  d=_mm_cvtsd_f64(di);
#elif defined(__aarch64__)
  const float64x1_t ai=vld1_f64(&a);
  const float64x1_t di=vsqrt_f64(ai);
  vst1_f64(&d, di);
#else
#error("arch missing")
#endif
  return d;
//#endif
}
