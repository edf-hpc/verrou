

#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

#include "../../interflop_verrou/vr_fma.hxx"

#include <cstdint>

int  main(int argc, char** argv){

  int nearnessNum=VERROU_NUM_NEARNESS;
  int size=100000000;
  //int size=1000000;

  //init data for summation of constant increment
  float init=100000;
  float inc=0.1;

  std::vector<vr_RoundingMode> roundingMode={VR_NEAREST, VR_UPWARD,VR_DOWNWARD,VR_RANDOM,VR_NEARNESS};
  int nbRUN=20;

  std::vector<int> nbRunTab={1, 1, 1, nbRUN, nbRUN};
  if(nearnessNum!=1){
    for(int i=0; i<4; i++){
      nbRunTab[i]=0;
    }
  }

  for(size_t i=0; i< roundingMode.size();i++){
    //init the generator for the stochastic inc data  : need to be there to get the same data between each rounding mode
    int nbRun=nbRunTab[i];

    for(int j=0; j<nbRun; j++){

      //open output filename
      std::string name=verrou_rounding_mode_name(roundingMode[i]);
      std::stringstream ss;
      if( roundingMode[i]== VR_NEARNESS){
	ss<< "_"<< nearnessNum;
      }
       ss<<"." << j;
      std::string jStr=ss.str();

      if(nbRun==1){
	jStr=std::string("");
      }

      std::ofstream outStream("nearness_study_"+name+jStr+ std::string(".out"));
      outStream<< std::setprecision(17);

      // init verrou backend
      void* context;
      interflop_verrou_init(&context);
      interflop_verrou_configure(roundingMode[i], context);
      uint64_t seed(j);
      verrou_set_seed (seed);

      //init acc
      float acc=init;

      //param to select iteration output
      int nextPower=1;
      float step=1.1;

      for(int i=0; i<=size; i++){//increment loop

	//compute increment for stochastic case

	if( i==nextPower or i==size){ // detection of output iteration
	  double ref= init + ((double)i)*inc;
	  double accError= (double) acc- ref;
	  outStream << i<<"\t"<<acc << "\t"<< accError<<"\t" << std::abs(accError/ref)<< std::endl;

	  nextPower= (int)(nextPower*step);
	  if(i==nextPower) nextPower=i+1;
	}

	//compute increment with interflop verrou backend
	float acc_new;
	interflop_verrou_add_float(acc,inc,&acc_new,context);

	//update accumulator
	acc=acc_new;
      }

      interflop_verrou_finalize(&context);
    }
  }
}
