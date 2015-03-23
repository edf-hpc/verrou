
#include <iostream>
#include <math.h>
#include <cstdlib>


#include  <immintrin.h>
//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  double a,b,c,d,e ;
  float af,bf,cf,df,ef ;
  if(argc==4){
    a=atof(argv[1]);af=a;
    b=atof(argv[2]);bf=b;
    c=atof(argv[3]);cf=c;
  }else{
    std::cerr << "demande 3 argument"<<std::endl;
    return EXIT_FAILURE;
  }
  bool fast;
  //#ifdef FP_FAST_FMA
  //  fast=true;
  //  d=fma(a,b,c);
  //#else
  fast=false;

  {
  __m128d ai, bi,ci,di,ei ;
  ai = _mm_load_sd(&a);
  bi = _mm_load_sd(&b);
  ei=_mm_add_sd(ai,bi);
  e=_mm_cvtsd_f64(ei);
  std::cout << "e computed" << std::endl; 
  }
  {
  d=a+b;
  std::cout << "resultat intri add: " << e << ","<< d<<std::endl;
  }
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
  //  d=di;
  //#endif

  
  std::cout << " a, b, c : " << a <<","<< b <<","<< c <<std::endl;
  std::cout << "resultat intri fma double: " << d << std::endl;
  std::cout << "resultat intri fma float: " << df << std::endl;



  return EXIT_SUCCESS;
}
