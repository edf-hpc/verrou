#include "../verrou.h"
#include <stdio.h>
#include <math.h>

#define NUM 20

float computeMaxFloat(float dat, float(*func)(float) ){
  float maxValue=0.;
  for(int i=0; i< NUM; i++){
    float value= func(dat) - func(dat);
    if(value<0){value=-value;}
    if(value>maxValue){maxValue=value;}    
  }
  return maxValue;
};

double computeMaxDouble(double dat, double(*func)(double) ){
  double maxValue=0.;
  for(int i=0; i< NUM; i++){
    double value= func(dat) -func(dat);
    if(value<0){value=-value;}
    if(value>maxValue){maxValue=value;}    
  }
  return maxValue;
};

float atan2fFunctor(float dat){
  return atan2f(dat, 41.f);
}

double atan2Functor(double dat){
  return atan2(dat, 41.);
}

void checkSupFloat(char* s, float v1, float v2){
  if(v1 > v2){
    printf("%s un/re-inst : OK\n",s);
  }else{
    printf("%s un/re-inst : KO\n",s);
  }
}

void checkSupDouble(char* s, double v1, double v2){
  if(v1 > v2){
    printf("%s un/re-inst : OK\n",s);
  }else{
    printf("%s un/re-inst : KO\n",s);
  }
}


int main () {

  float resCosf=computeMaxFloat(42.f,cosf);
  float resAtan2f=computeMaxFloat(42.,atan2fFunctor);
  double resCos=computeMaxDouble(42.,cos);
  double resAtan2=computeMaxDouble(42.,atan2Functor);
  
  // Uninstrumented part
  VERROU_STOP_INSTRUMENTATION;

  float resCosf_UNINST  =computeMaxFloat(42.f,cosf);
  float resAtan2f_UNINST=computeMaxFloat(42.,atan2fFunctor);
  double resCos_UNINST  =computeMaxDouble(42.,cos);
  double resAtan2_UNINST=computeMaxDouble(42.,atan2Functor);

  checkSupFloat("cosf",resCosf, resCosf_UNINST);
  checkSupFloat("atan2f",resAtan2f, resAtan2f_UNINST);
  checkSupDouble("cos",resCos, resCos_UNINST);
  checkSupDouble("atan2",resAtan2, resAtan2_UNINST);

  VERROU_START_INSTRUMENTATION;

  float resCosf_REINST  =computeMaxFloat(42.f,cosf);
  float resAtan2f_REINST=computeMaxFloat(42.,atan2fFunctor);
  double resCos_REINST  =computeMaxDouble(42.,cos);
  double resAtan2_REINST=computeMaxDouble(42.,atan2Functor);

  checkSupFloat("cosf",  resCosf_REINST,   resCosf_UNINST);
  checkSupFloat("atan2f",resAtan2f_REINST, resAtan2f_UNINST);
  checkSupDouble("cos"  ,resCos_REINST,    resCos_UNINST);
  checkSupDouble("atan2",resAtan2_REINST,  resAtan2_UNINST);
  
  return RUNNING_ON_VALGRIND;
}
