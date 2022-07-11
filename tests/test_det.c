#include "../verrou.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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
  for (i = 0 ; i < N ; ++i) {
    sum0 += epsilon * (0.1* epsilon);
    sum1 += (0.1*epsilon) * (epsilon);
  }
  return sum0-sum1;
};


int main (int argc, char **argv) {
  float diff_det    = compute_det();
  float diff_comdet = compute_comdet();

  if(diff_det==0.0){
    printf ("DET\n");
  }else{
    printf ("NO DET\n");
  }

  if(diff_comdet==0.0){
    printf ("COMDET\n");
  }else{
    printf ("NO COMDET\n");
  }

  return 0;
}
