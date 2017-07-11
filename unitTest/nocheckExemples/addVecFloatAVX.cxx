
#include <iostream>
#include <math.h>
#include <cstdlib>


#include  <immintrin.h>
#include <avxintrin.h>

int main(int argc, char** argv){
  float a[8]  __attribute__ ((aligned(32)));
  float b[8]  __attribute__ ((aligned(32)));
  float r[8]  __attribute__ ((aligned(32)));
    
  if(argc==17){
    a[0]=atof(argv[1]);
    a[1]=atof(argv[2]);
    a[2]=atof(argv[3]);
    a[3]=atof(argv[4]);
    a[4]=atof(argv[5]);
    a[5]=atof(argv[6]);
    a[6]=atof(argv[7]);
    a[7]=atof(argv[8]);


    b[0]=atof(argv[9]);
    b[1]=atof(argv[10]);
    b[2]=atof(argv[11]);
    b[3]=atof(argv[12]);
    b[4]=atof(argv[13]);
    b[5]=atof(argv[14]);
    b[6]=atof(argv[15]);
    b[7]=atof(argv[16]);

  }else{
    std::cerr << "demande 16 arguments : "<< argc-1 <<std::endl;
    return EXIT_FAILURE;
  }

  {
    __m256 ai, bi,ri;
  ai = _mm256_load_ps(a);
  bi = _mm256_load_ps(b);
  ri=_mm256_add_ps(ai,bi);
  _mm256_store_ps(r,ri);
  }
  

    std::cout.precision(10);
  std::cout << " a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7] " << a[0] <<","<< a[1]<<","<< a[2]<<","<< a[3] <<"," << a[4] <<","<< a[5]<<","<< a[6]<<","<< a[7] <<std::endl 
	    << " b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7] " << b[0] <<","<< b[1]<<","<< b[2]<<","<< b[3] <<"," << b[4] <<","<< b[5]<<","<< b[6]<<","<< b[7] <<std::endl 
    	    << " r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7] " << r[0] <<","<< r[1]<<","<< r[2]<<","<< r[3] <<"," << r[4] <<","<< r[5]<<","<< r[6]<<","<< r[7] <<std::endl 
	    << "  diff1-0 : "<<   (r[1]-r[0])<<std::endl
  	    << "  diff2-0 : "<<   (r[2]-r[0])<<std::endl
	    << "  diff3-0 : "<<   (r[3]-r[0])<<std::endl
      	    << "  diff4-0 : "<<   (r[4]-r[0])<<std::endl
      	    << "  diff5-0 : "<<   (r[5]-r[0])<<std::endl
      	    << "  diff6-0 : "<<   (r[6]-r[0])<<std::endl
      	    << "  diff7-0 : "<<   (r[7]-r[0])<<std::endl  ;




  return EXIT_SUCCESS;
}
