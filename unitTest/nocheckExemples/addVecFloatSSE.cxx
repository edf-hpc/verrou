
#include <iostream>
#include <math.h>
#include <cstdlib>


#include  <immintrin.h>
//#include <avxintrin_emu.h>
int main(int argc, char** argv){
  float a[4] ,b[4] ;
  float r[4];
    
  if(argc==9){
    a[0]=atof(argv[1]);
    a[1]=atof(argv[2]);
    a[2]=atof(argv[3]);
    a[3]=atof(argv[4]);
    b[0]=atof(argv[5]);
    b[1]=atof(argv[6]);
    b[2]=atof(argv[7]);
    b[3]=atof(argv[8]);

    
  }else{
    std::cerr << "demande 8 arguments"<<std::endl;
    return EXIT_FAILURE;
  }

  {
    __m128 ai, bi,ri;
  ai = _mm_load_ps(a);
  bi = _mm_load_ps(b);
  ri=_mm_add_ps(ai,bi);
  _mm_store_ps(r,ri);
  }
  

  
  std::cout.precision(10);
  std::cout << " a[0], a[1], a[2], a[3] " << a[0] <<","<< a[1]<<","<< a[2]<<","<< a[3] <<std::endl 
	    << " b[0], b[1], b[2], b[3] " << b[0] <<","<< b[1]<<","<< b[2]<<","<< b[3] <<std::endl 
	    << " r[0], r[1], r[2], r[3] " << r[0] <<","<< r[1]<<","<< r[2]<<","<< r[3] <<std::endl 
	    << "  diff1-0"<<   (r[1]-r[0])<<std::endl
  	    << "  diff2-0"<<   (r[2]-r[0])<<std::endl
	    << "  diff3-0"<<   (r[3]-r[0])<<std::endl;




  return EXIT_SUCCESS;
}
