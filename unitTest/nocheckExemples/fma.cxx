
#include <iostream>
#include <math.h>
#include <cstdlib>
#ifdef __x86_64__
#include  <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif


//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  double a,b,c,d,e ;
  float af,bf,cf,df,ef ;
  if(argc==4){
    a=atof(argv[1]);af=a;
    b=atof(argv[2]);bf=b;
    c=atof(argv[3]);cf=c;
  }else{
    std::cerr << "requiere 3 arguments"<<std::endl;
    return EXIT_FAILURE;
  }

#ifdef __x86_64__
  {
    __m128d ai, bi,ci,di,ei ;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmadd_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);
  }

  {
    __m128 ai, bi,ci,di,ei ;
    ai = _mm_load_ss(&af);
    bi = _mm_load_ss(&bf);
    ci = _mm_load_ss(&cf);
    di=_mm_fmadd_ss(ai,bi,ci);
    df=_mm_cvtss_f32(di);
  }
#endif

#ifdef __aarch64__
  {
    const float64x1_t ai=vld1_f64(&a);
    const float64x1_t bi=vld1_f64(&b);
    const float64x1_t ci=vld1_f64(&c);

    const float64x1_t di=vfma_f64(ci,ai,bi);
    vst1_f64(&d, di);
  }
  {
    float av[2]={af,0};
    float bv[2]={bf,0};
    float cv[2]={cf,0};

    float32x2_t ap=vld1_f32(av);
    float32x2_t bp=vld1_f32(bv);
    float32x2_t cp=vld1_f32(cv);

    float32x2_t resp= vfma_f32(cp,ap,bp);
    float res[2];
    vst1_f32(res, resp);
    df=res[0];
  }
#endif

  double libmFMA=fma(a,b,c);
  float libmFMAf=fmaf(af,bf,cf);

  std::cout << " a, b, c : " << a <<","<< b <<","<< c <<std::endl;
  std::cout << "resultat intri fma double: " << d << std::endl;
  std::cout << "resultat intri fma float: " << df << std::endl;

  std::cout << "fma double diff intri/lib: " << d -libmFMA << std::endl;
  std::cout << "fma float diff intri/lib: " << df -libmFMAf<< std::endl;

  return EXIT_SUCCESS;
}
