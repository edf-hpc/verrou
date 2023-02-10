
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <../verrou.h>

int N = 100000;
float epsilon = 1.e-6;

float compute_det (void) {
  float sum0 = 0.0;
  float sum1 = 0.0;
  int i;
  for (i = 0 ; i < N ; ++i) {
    sum0 += epsilon * epsilon;
    sum1 += epsilon * epsilon;
  }
  return sum0-sum1;
};

float compute_comdet (void) {
  float sum0=0.;
  float sum1=0.;
  int i;
  float factor1=0.1;
  float factor2=0.2;

  for (i = 0 ; i < N ; ++i) {
    sum0 = sum0+factor1*factor2;
    sum1 = factor2*factor1 + sum1;
  }
  return sum0-sum1;
};

float compute_scomdet (void) {
  float sum0=0.;
  float sum1=0.;
  int i;

  float factor1=0.1;
  float factor2=0.2;
  for (i = 0 ; i < N ; ++i) {
    sum0 += factor1 *factor2;
    sum1 += ((-factor1) * factor2);
  }
  return sum0+sum1;
};


float compute_monotonic(void){
  float sum0=1.;
  float sum1=1.;

  float inc0=1e-8;
  float inc1=2e-8;
  sum0+=inc0;
  sum1+=inc1;

  return sum1 -sum0;
}




int main (int argc, char **argv) {

  bool isDet=true;
  bool isComDet=true;
  bool isSComDet=true;
  bool isMonotonic=true;
  for(int i=0; i<50; i++){
    VERROU_SET_SEED(i);
    float diff_det    = compute_det();
    float diff_comdet = compute_comdet();
    float diff_scomdet = compute_scomdet();
    float diff_monotonic = compute_monotonic();
    isDet=isDet &&(diff_det==0.);
    isComDet=isComDet &&(diff_comdet==0.);
    isSComDet=isSComDet &&(diff_scomdet==0.);
    isMonotonic=isMonotonic && (diff_monotonic>=0);
  }

  if(isDet){
    printf ("DET\n");
  }else{
    printf ("NO DET\n");
  }

  if(isComDet){
    printf ("COMDET\n");
  }else{
    printf ("NO COMDET\n");
  }

  if(isSComDet){
    printf ("SCOMDET\n");
  }else{
    printf ("NO SCOMDET\n");
  }

  if(isMonotonic){
    printf ("MONOTONIC\n");
  }else{
    printf ("NO MONOTONIC\n");
  }


  return 0;
}
