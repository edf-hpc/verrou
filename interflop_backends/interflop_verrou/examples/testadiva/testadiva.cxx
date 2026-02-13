

#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <math.h> 
#include <cassert>
#include "tinymt64.h"


int  main(int argc, char** argv){
  int size=100000000;
  const double tol=1.e-4;
  const double a(1-pow(2,30));
  const double one(1.);
  const double ref(1.);
  const double refm(nextafter(ref, 0.));
  const double refp(nextafter(ref, 2.));
      
  
  std::vector<vr_RoundingMode> roundingMode={VR_RANDOM, VR_NEARNESS,  VR_SR_MONOTONIC,  VR_NEAREST};

  tinymt64_t rng;
  //init the generator for seed update (required for stochastic/determinist rounding mode)
  tinymt64_init(&rng, 0);


  for(size_t rnd=0; rnd< roundingMode.size();rnd++){
    
    void* context;
    interflop_verrou_init(&context);
    interflop_verrou_configure(roundingMode[rnd], context);
    uint64_t seed(41121);
    verrou_set_seed (seed);
    
    std::cout << verrou_rounding_mode_name(roundingMode[rnd])<<std::endl;
    
    int countm=0;
    int counte=0;
    int countp=0;
    int localSize=size;
    if(roundingMode[rnd]==VR_NEAREST){
      localSize=1;
    }
    
    for(int i=0; i<localSize; i++){//increment loop
      double diva; 
      double adiva; 

      if(roundingMode[rnd]==VR_SR_MONOTONIC){
	verrou_set_seed(tinymt64_generate_uint64 (&rng));
      }

      
      interflop_verrou_div_double(one,a,&diva,context);
      interflop_verrou_mul_double(diva,a,&adiva,context);

      
      if(adiva==refm) {
	countm++;
      }else{	
	if(adiva==ref){
	  counte++;
	}else{
	  if(adiva==refp){
	    countp++;
	  }else{
	    std::cerr << "unexpected value"<<std::endl;	    
	  }
	  
	}
	
      }
    }
    double pm((double)countm / (double)localSize);
    double pe((double)counte / (double)localSize);
    double pp((double)countp / (double)localSize);



    interflop_verrou_finalize(&context);
    std::cout << std::setprecision(17);
    std::cout << "\t"<<refm<<" :\t"<< pm  <<std::endl;
    std::cout << "\t"<<ref<<" :\t"<<  pe <<std::endl;
    std::cout << "\t"<<refp<<" :\t"<< pp <<std::endl;


    if(roundingMode[rnd]==VR_NEAREST){
      assert(pm==0.);
      assert(std::abs(pe-1.) <1e-15 );
      assert(pp==0.);
    }

    if(roundingMode[rnd]==VR_RANDOM){
      assert(std::abs(pm-0.25) <tol );
      assert(std::abs(pe-0.5) <tol );
      assert(std::abs(pp-0.25) <tol );
    }

    if(roundingMode[rnd]==VR_NEARNESS || roundingMode[rnd]==VR_SR_MONOTONIC ){
      double pmref(pow(2,-7) - pow(2,-15)- (1./ a) *pow(2,-45) );
      double ppref(pow(2,-8) - pow(2,-16)-  pow(2,-38) +  (1./ a) *pow(2,-38)- (1./ a) *pow(2,-46)- (1./ a) *pow(2,-68));
      double peref=1- pmref- ppref;
	
      assert(std::abs(pm-pmref) <tol );
      assert(std::abs(pe-peref) <tol);
      assert(std::abs(pp-ppref) <tol );

    
    }
  }
}
