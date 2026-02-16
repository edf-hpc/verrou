

#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <math.h>
#include <quadmath.h>
#include <cassert>
#include "tinymt64.h"
#include <cfloat>

int  main(int argc, char** argv){
  int size=1000000;
  //int size=100;
  const double tol=1.e-3;
  const double a(2.);
  const double ref(a);

  const double am=nextafter(a,0);
  const double amm=nextafter(am,0);
  const double ap=nextafter(a,4);


  std::vector<vr_RoundingMode> roundingMode={VR_RANDOM, VR_NEAREST, VR_DOWNWARD, VR_NEARNESS, VR_SR_MONOTONIC, VR_SR_SMONOTONIC };

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

    int cmm=0;
    int cm=0;
    int ce=0;
    int cp=0;
    int localSize=size;
    if(roundingMode[rnd]==VR_NEAREST ||  roundingMode[rnd]==VR_DOWNWARD){
      localSize=1;
    }

    for(int i=0; i<localSize; i++){//increment loop
      //      std::cout << "i: "<< i<< "\tlocalSize: "<< localSize<<std::endl;
      double sqrta1;
      double sqrta2;
      double sqrtaSquare;

      if(roundingMode[rnd]==VR_SR_MONOTONIC || roundingMode[rnd]==VR_SR_SMONOTONIC){
	verrou_set_seed(tinymt64_generate_uint64 (&rng));
      }

      interflop_verrou_sqrt_double(a,&sqrta1,context);
      interflop_verrou_sqrt_double(a,&sqrta2,context);

      interflop_verrou_mul_double(sqrta1,sqrta2,&sqrtaSquare,context);

      int check=0;
      if(sqrtaSquare==amm){cmm++;check++;}
      if(sqrtaSquare==am ) {cm++;check++;}
      if(sqrtaSquare==ref ) {ce++;check++;}
      if(sqrtaSquare==ap ) {cp++;check++;}
      if(check!=1){
	__float128 sqrtaq;
	sqrtaq=sqrtq((__float128)a);
	__float128 errorq1(sqrta1 -sqrtaq);
	errorq1=(errorq1/ sqrtaq) / DBL_EPSILON;
	std::cout<< "signed error" <<(double)errorq1<<std::endl;

	double error=((sqrtaSquare -ref) /ref)/ DBL_EPSILON;
	std::cout << "error:" << error  << std::endl;
	std::cerr << "unexpected value  error: " << error << std::endl;
      }

    }
    float pmm( cmm / (float)localSize);
    float pm( cm / (float)localSize);
    float pe( ce / (float)localSize);
    float pp( cp / (float)localSize);

    interflop_verrou_finalize(&context);



    if(roundingMode[rnd]==VR_NEAREST){
      //      assert(pm==0.);
      //      assert(std::abs(pe-1.) <1e-15 );
      //      assert(pp==0.);
    }

    if(roundingMode[rnd]==VR_RANDOM){
      std::cout << "pmm: "<< pmm<<std::endl;
      std::cout << "pm: "<< pm<<std::endl;
      std::cout << "pe: "<< pe<<std::endl;
      std::cout << "pp: "<< pp<<std::endl;

      // assert(std::abs(pmm-0.25) <tol );
      // assert(std::abs(pm-0.25) <tol );
      // assert(std::abs(pe-0.25) <tol );
      // assert(std::abs(pp-0.25) <tol );
    }

    if(roundingMode[rnd]==VR_NEARNESS || roundingMode[rnd]==VR_SR_MONOTONIC || roundingMode[rnd]==VR_SR_SMONOTONIC ){
      std::cout << "pmm: "<< pmm<<std::endl;
      std::cout << "pm: "<< pm<<std::endl;
      std::cout << "pe: "<< pe<<std::endl;
      std::cout << "pp: "<< pp<<std::endl;

    }
  }
}
