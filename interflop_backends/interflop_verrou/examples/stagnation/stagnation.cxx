

#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

#include  "tinymt64.h"



int  main(int argc, char** argv){

  int size=100000000;

  //init data for summation of constant increment
  float init=100000;
  float inc=0.1;

  //init data for summation of random increment (between 0 2)
  float init_st=10000000;
  tinymt64_t rng;

  std::vector<vr_RoundingMode> roundingMode={VR_NEAREST, VR_RANDOM, VR_RANDOM_DET,VR_AVERAGE, VR_AVERAGE_DET};


  for(size_t i=0; i< roundingMode.size();i++){
    //init the generator for the stochastic inc data  : need to be there to get the same data between each rounding mode
    tinymt64_init(&rng, 0);

    //open output filename
    std::string name=verrou_rounding_mode_name(roundingMode[i]);
    std::ofstream outStream(name+std::string(".out"));
    outStream<< std::setprecision(17);
    std::ofstream outStreamSt(name+std::string("_st.out"));
    outStreamSt<< std::setprecision(17);

    // init verrou backend
    void* context;
    interflop_verrou_init(&context);
    interflop_verrou_configure(roundingMode[i], context);
    uint64_t seed(42);
    verrou_set_seed (seed);

    //init acc
    float acc=init;
    float acc_st=init_st;
    __float128 ref_st=init_st; // to compute reference in stochastic data case

    //variable to detect stagnation
    int stagnation=-1;
    int stagnation_st=-1;
    int nb_stagnation=0;
    int nb_stagnation_st=0;

    //param to select iteration output
    int nextPower=1;
    float step=1.1;

    for(int i=0; i<=size; i++){//increment loop

      //compute increment for stochastic case
      double rnd0_1=tinymt64_generate_double(&rng);
      float inc_st=(rnd0_1*2.);

      if( i==nextPower or i==size){ // detection of output iteration
	double ref= init + ((double)i)*inc;
	double accError= (double) acc- ref;
	outStream << i<<"\t"<<acc << "\t"<< accError<<"\t" << std::abs(accError/ref)<< std::endl;

	__float128 accError_st= (__float128)acc_st-ref_st;
	outStreamSt << i<<"\t"<< (double)acc_st << "\t"<< (double)accError_st<<"\t" << (double)std::abs(accError_st/ref_st)<< "\t"<< (double)ref_st <<std::endl;

	nextPower= (int)(nextPower*step);
	if(i==nextPower) nextPower=i+1;
      }

      //compute increment with interflop verrou backend
      float acc_new;
      interflop_verrou_add_float(acc,inc,&acc_new,context);
      float acc_new_st;
      interflop_verrou_add_float(acc_st,inc_st,&acc_new_st,context);

      ref_st+=(__float128)inc_st; //compute the reference


      //detect stagnation
      if(acc_new==acc && stagnation==-1){
	stagnation=i;
      }
      if(acc_new==acc && stagnation!=-1){
	nb_stagnation+=1;
      }
      if(stagnation!=-1 && acc_new!=acc){
	stagnation=-1;
	nb_stagnation=0;
      }
      if(acc_new_st==acc_st && stagnation_st==-1){
	stagnation_st=i;
      }
      if(acc_new_st==acc_st && stagnation_st!=-1){
	nb_stagnation+=1;
      }
      if(stagnation_st!=-1 && acc_new_st!=acc_st){
	stagnation_st=-1;
	nb_stagnation_st=0;
      }

      //update accumulator
      acc=acc_new;
      acc_st=acc_new_st;
    }

    if(nb_stagnation>10){//need to be manually checked
      std::ofstream outStaStream(name+std::string(".STAGNATION.out"));
      outStaStream <<stagnation <<std::endl;
    }

    if(nb_stagnation_st>10){//need to be manually checked
      std::ofstream outStaStream(name+std::string(".STAGNATION_st.out"));
      outStaStream <<stagnation_st <<std::endl;
    }
  }
}
