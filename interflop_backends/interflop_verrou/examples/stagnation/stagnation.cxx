

#include <iostream>
#include "../../interflop_verrou.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>



int  main(int argc, char** argv){
  float init=100000;
  float inc=0.1;

  std::vector<vr_RoundingMode> roundingMode={VR_NEAREST, VR_RANDOM, VR_RANDOM_DET,VR_AVERAGE, VR_AVERAGE_DET};



  for(size_t i=0; i< roundingMode.size();i++){
    std::string name=verrou_rounding_mode_name(roundingMode[i]);
    std::ofstream outStream(name+std::string(".out"));
    outStream<< std::setprecision(17);
    void* context;
    struct interflop_backend_interface_t ifverrou=interflop_verrou_init(&context);
    interflop_verrou_configure(roundingMode[i], context);
    uint64_t seed(42);
    verrou_set_seed (seed);

    float acc=init;
    int stagnation=-1;
    int nb_stagnation=0;
    int nextPower=1;
    float step=1.1;
    for(int i=0; i<=100000000; i++){
      if( i==nextPower){
	double ref= init + ((double)i)*inc;
	double accError= (double) acc- ref;
	outStream << i<<"\t"<<acc << "\t"<< accError<<"\t" << std::abs(accError/ref)<< std::endl;
	nextPower= (int)(nextPower*step);
	if(i==nextPower) nextPower=i+1;
      }
      float acc_new;
      interflop_verrou_add_float(acc,inc,&acc_new,context);
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
      acc=acc_new;
    }
    if(nb_stagnation>10){
      std::ofstream outStaStream(name+std::string(".STAGNATION.out"));
      outStaStream <<stagnation <<std::endl;
    }
  }
}
