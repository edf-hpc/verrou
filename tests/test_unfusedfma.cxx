#include "../verrou.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string>
#include <iostream>
#include "fma_common.hxx"

double a=0.1,b=0.1,c=-0.01;
float a_f=(float)a;
float b_f=(float)b;
float c_f=(float)c;


double ref_unfused;
double ref_fused;
float ref_unfused_f;
float ref_fused_f;
double unfused;
double fused;
float unfused_f;
float fused_f;

void computeRef(){
  ref_unfused=a*b+c;
  ref_fused=intrin_fma<double>(a,b,c);

  ref_unfused_f=a_f*b_f+c_f;
  ref_fused_f=intrin_fma<float>(a_f,b_f,c_f);
}

template<class T1, class T2>
std::string equal(T1& x,T2& y){
  if(x==y){
    return std::string("==");
  }else{
    return std::string("!=");
  }
}

void compute(){
  unfused=a*b+c;
  fused=intrin_fma<double>(a,b,c);

  unfused_f=a_f*b_f+c_f;
  fused_f=intrin_fma<float>(a_f,b_f,c_f);

  std::cout << "fused" << equal(fused, unfused)<<"unfused"<<std::endl;
  std::cout << "fused_f" << equal(fused_f, unfused_f)<<"unfused_f"<<std::endl;
  std::cout << "fused" << equal(fused, fused_f)<<"fused_f"<<std::endl;
  std::cout << "unfused" << equal(unfused, unfused_f)<<"unfused_f"<<std::endl;

  //std::cout << "fused" << equal(fused, ref_fused) << "ref_fused"<<std::endl;
  //  std::cout << "fused_f" << equal(fused_f, ref_fused_f) << "ref_fused_f"<<std::endl;
  //std::cout << "unfused" << equal(unfused, ref_unfused) << "ref_unfused"<<std::endl;
  //std::cout << "unfused_f" << equal(unfused_f, ref_unfused_f) << "ref_unfused_f"<<std::endl;

  std::cout << std::endl;
}




int main (int argc, char** argv) {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

  //computeRef();
  compute();

  VERROU_STOP_SOFT_INSTRUMENTATION;
  VERROU_START_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_STOP_INSTRUMENTATION;
  VERROU_START_SOFT_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_START_INSTRUMENTATION;
  std::cout << "inst"<<std::endl;
  compute();//inst

  VERROU_STOP_SOFT_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_START_SOFT_INSTRUMENTATION;
  std::cout << "inst"<<std::endl;
  compute();//inst

  VERROU_STOP_SOFT_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_STOP_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_START_INSTRUMENTATION;
  std::cout << "non inst"<<std::endl;
  compute();//non inst

  VERROU_START_SOFT_INSTRUMENTATION;
  std::cout << "inst"<<std::endl;
  compute();//inst

  VERROU_START_SOFT_INSTRUMENTATION;
  std::cout << "inst"<<std::endl;
  compute();//inst

  return RUNNING_ON_VALGRIND;
}
