#pragma once

#ifdef    USE_VERROU_FMA
#include  <immintrin.h>
#include  <fmaintrin.h>

template<class REALTYPE> 
inline REALTYPE vr_fma(const REALTYPE&, const REALTYPE&, const REALTYPE&){
  return 0./ 0.; //nan to be sur not used
} 

template<>
inline double vr_fma<double>(const double& a, const double& b, const double& c){
  double d;
  __m128d ai, bi,ci,di;
  ai = _mm_load_sd(&a);
  bi = _mm_load_sd(&b);
  ci = _mm_load_sd(&c);
  di=_mm_fmadd_sd(ai,bi,ci);
  d=_mm_cvtsd_f64(di);
  return d;
}


template<>
inline float vr_fma<float>(const float& a, const float& b, const float& c){
  float d;
  __m128 ai, bi,ci,di;
  ai = _mm_load_ss(&a);
  bi = _mm_load_ss(&b);
  ci = _mm_load_ss(&c);
  di=_mm_fmadd_ss(ai,bi,ci);
  d=_mm_cvtss_f32(di);
  return d;
}
#endif //USE_VERROU_FMA


