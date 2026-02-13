
#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <math.h>
#include <quadmath.h>
#include <cassert>
#include "tinymt64.h"
#include <cfloat>
#include <cmath>
#include <cstdlib>

template<class REALTYPE>
REALTYPE my_abs(const REALTYPE & a){
  return (a>=0 ? a :-a);
}

void printNan(){
  //  std::cerr << "Nan detected"<<std::endl;
  exit(42);
}


bool isDet(vr_RoundingMode rnd){
  if(rnd==VR_NEAREST) return true;
  if(rnd==VR_UPWARD)  return true;
  if(rnd==VR_DOWNWARD) return true;
  if(rnd==VR_FARTHEST) return true;
  return false;
}


bool isRandomLike(vr_RoundingMode rnd){
  if(rnd==VR_RANDOM) return true;
  if(rnd==VR_RANDOM_DET) return true;
  if(rnd==VR_RANDOM_COMDET) return true;
  if(rnd==VR_RANDOM_SCOMDET) return true;
  if(rnd==VR_PRANDOM) return true;
  return false;
}

bool isNearnessLike(vr_RoundingMode rnd){
  if(rnd==VR_NEARNESS) return true;
  if(rnd==VR_NEARNESS_DET) return true;
  if(rnd==VR_NEARNESS_COMDET) return true;
  if(rnd==VR_NEARNESS_SCOMDET) return true;
  if(rnd==VR_SR_MONOTONIC) return true;
  return false;
}

bool needToUpdateSeed(vr_RoundingMode rnd){
  if(rnd==VR_RANDOM_DET) return true;
  if(rnd==VR_RANDOM_COMDET) return true;
  if(rnd==VR_RANDOM_SCOMDET) return true;
  if(rnd==VR_NEARNESS_DET) return true;
  if(rnd==VR_NEARNESS_COMDET) return true;
  if(rnd==VR_NEARNESS_SCOMDET) return true;
  if(rnd==VR_SR_MONOTONIC) return true;
  if(rnd==VR_PRANDOM) return true;
  return false;
}


int  main(int argc, char** argv){
  int size=1000000;
  //  int size=6;
  const double tol=1.e-3;
  const double a(1.e-200);
  const double b(1.e-200);

  const double resm=0.;
  const double resp=nextafter(0.,1.);



  std::vector<vr_RoundingMode> roundingMode= {
    VR_NEAREST,VR_DOWNWARD, VR_UPWARD, VR_FARTHEST,
    VR_RANDOM, VR_RANDOM_DET, VR_RANDOM_COMDET,  VR_RANDOM_SCOMDET,
    VR_NEARNESS, VR_NEARNESS_DET, VR_NEARNESS_COMDET,  VR_NEARNESS_SCOMDET,
    VR_PRANDOM,
    VR_SR_MONOTONIC };

  tinymt64_t rng;
  //init the generator for seed update (required for stochastic/determinist rounding mode)
  tinymt64_init(&rng, 0);


  for(size_t rnd=0; rnd< roundingMode.size();rnd++){

    void* context;
    interflop_verrou_init(&context);
    interflop_verrou_configure(roundingMode[rnd], context);
    uint64_t seed(41121);
    verrou_set_seed (seed);
    verrou_set_nan_handler(printNan);

    std::cout << verrou_rounding_mode_name(roundingMode[rnd])<<std::endl;

    int cm=0;
    int cp=0;
    int localSize=size;
    if(isDet(roundingMode[rnd])){
      localSize=1;
    }

    for(int i=0; i<localSize; i++){//increment loop
      //      std::cout << "i: "<< i<< "\tlocalSize: "<< localSize<<std::endl;
      double res;

      if(needToUpdateSeed(roundingMode[rnd])){
	verrou_set_seed(tinymt64_generate_uint64 (&rng));
      }

      interflop_verrou_mul_double(a,b, &res,context);
      int check=0;
      if(res==resm){cm++;check++;}
      if(res==resp ){cp++;check++;}
      if(check!=1){
	//	__float128 resq;
	//	resq=(__float128)a) * (__float128)b);
	//	__float128 error(res -resq);
	std::cout << "Unexpected value : " << res<< std::endl;
      }
    }
    float pm( cm / (float)localSize);
    float pp( cp / (float)localSize);

    interflop_verrou_finalize(&context);

    std::cout << "pm: "<< pm<<std::endl;
    std::cout << "pp: "<< pp<<std::endl;

    if(roundingMode[rnd]==VR_NEAREST){
      assert(pm==1.);
      assert(pp==0.);
    }
    if(roundingMode[rnd]==VR_UPWARD){
#if VERROU_DENORM_HACKS_DOUBLE
      assert(pm==0.);
      assert(pp==1.);
#endif
    }
    if(roundingMode[rnd]==VR_DOWNWARD){
      assert(pm==1.);
      assert(pp==0.);
    }

    if(isRandomLike(roundingMode[rnd])){
#if VERROU_DENORM_HACKS_DOUBLE
      assert(my_abs<float>(pm-0.5) <tol );
      assert(my_abs<float>(pp-0.5) <tol );
#endif
    }
    if(isNearnessLike(roundingMode[rnd])){
      assert(my_abs<float>(pm-1.) < tol );
      assert(my_abs<float>(pp-0.) < tol );
    }
  }
}
