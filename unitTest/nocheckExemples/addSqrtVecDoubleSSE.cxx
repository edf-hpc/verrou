
#include <iostream>
#include <math.h>
#include <cstdlib>


#include  <immintrin.h>
//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  double a[2] ,b[2] ;
  double r[2];
    
  if(argc==5){
    a[0]=atof(argv[1]);
    a[1]=atof(argv[2]);
    b[0]=atof(argv[3]);
    b[1]=atof(argv[4]);    
  }else{
    std::cerr << "demande 4 argument"<<std::endl;
    return EXIT_FAILURE;
  }

  {
    __m128d ai, bi,ri;
  ai = _mm_load_pd(a);
  bi = _mm_load_pd(b);
  ri=_mm_add_pd(ai,bi);
  ri=_mm_sqrt_pd(ri);

  _mm_store_pd(r,ri);
  }
  

  
  std::cout << " a[0], a[1] " << a[0] <<","<< a[1] <<std::endl 
	    << " b[0], b[1] " << b[0] <<","<< b[1] <<std::endl 
	    << " r[0], r[1] "  << r[0] <<","<< r[1] <<std::endl 
	    << "  diff"<<   (r[1]-r[0])<<std::endl;




  return EXIT_SUCCESS;
}
