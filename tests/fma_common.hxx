#pragma once
#if defined(VGA_amd64) || defined(__x86_64__)
#include  <immintrin.h>
#endif

#if defined(VGA_arm64) || defined(__aarch64__)
#include "arm_neon.h"
#endif


template<class REALTYPE>
inline REALTYPE intrin_fma(const REALTYPE&, const REALTYPE&, const REALTYPE&);

#if defined(__x86_64__)
template<>
inline double intrin_fma<double>(const double& a, const double& b, const double& c){
  double d;
  __m128d ai, bi,ci,di;
  ai = _mm_load_sd(&a);
  bi = _mm_load_sd(&b);
  ci = _mm_load_sd(&c);
  di=_mm_fmadd_sd(ai,bi,ci);
  d=_mm_cvtsd_f64(di);
  return d;
}
#endif

#if defined(__aarch64__)
template<>
inline double intrin_fma<double>(const double& a, const double& b, const double& c){
  double d;
  const float64x1_t ai=vld1_f64(&a);
  const float64x1_t bi=vld1_f64(&b);
  const float64x1_t ci=vld1_f64(&c);

  const float64x1_t di=vfma_f64(ci,ai,bi);// warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  vst1_f64(&d, di);
  return d;
}
#endif



#if defined(__x86_64__)
template<>
inline float intrin_fma<float>(const float& a, const float& b, const float& c){
  float d;
  __m128 ai, bi,ci,di;
  ai = _mm_load_ss(&a);
  bi = _mm_load_ss(&b);
  ci = _mm_load_ss(&c);
  di=_mm_fmadd_ss(ai,bi,ci);
  d=_mm_cvtss_f32(di);
  return d;
}
#endif


#if defined(__aarch64__)
template<>
inline float intrin_fma<float>(const float& a, const float& b, const float& c){
  float res;
  __asm__(
	  "fmadd %s0, %s1, %s2, %s3":
	  "=x"(res): "x"(a),"x"(b),"x"(c)
	  );
  return res;
}
#endif
