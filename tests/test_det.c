
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



int main (int argc, char **argv) {

  bool isDet=true;
  bool isComDet=true;
  bool isSComDet=true;
  for(int i=0; i<50; i++){
    VERROU_SET_SEED(i);
    float diff_det    = compute_det();
    float diff_comdet = compute_comdet();
    float diff_scomdet = compute_scomdet();
    isDet=isDet &&(diff_det==0.);
    isComDet=isComDet &&(diff_comdet==0.);
    isSComDet=isSComDet &&(diff_scomdet==0.);
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

  
  return 0;
}
