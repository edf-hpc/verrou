
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <../verrou.h>

int N = 100000;


template<class REALTYPE>
struct myEpsilon;

template<>
struct myEpsilon<double>{
  static constexpr double value=1.e-15;
};
template<>
struct myEpsilon<float>{
  static constexpr float value=1.e-6;
};

template<class REALTYPE>
struct myUnderEpsilon;

template<>
struct myUnderEpsilon<double>{
  static constexpr double value=2.e-17;
};
template<>
struct myUnderEpsilon<float>{
  static constexpr float value=2.e-8;
};



template<class REALTYPE>
class test_compute{
public:
  typedef REALTYPE Realtype;
  test_compute(){
  };
  
  void run(){
    bool isDet=true;
    bool isComDet=true;
    bool isSComDet=true;
    bool isMonotonic=true;
    for(int i=0; i<50; i++){
      VERROU_SET_SEED(i);
      Realtype diff_det    = compute_det();
      Realtype diff_comdet = compute_comdet();
      Realtype diff_scomdet = compute_scomdet();
      Realtype diff_monotonic = compute_monotonic();
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
  }

  

private:
  
  Realtype compute_det (void) {
    Realtype sum0 = 0.0;
    Realtype sum1 = 0.0;
    int i;
    Realtype epsilon(myEpsilon<Realtype>::value);
    for (i = 0 ; i < N ; ++i) {
      sum0 += epsilon * epsilon;
      sum1 += epsilon * epsilon;
    }
    return sum0-sum1;
  };
  
  Realtype compute_comdet (void) {
    Realtype sum0=0.;
    Realtype sum1=0.;
    int i;
    Realtype factor1=0.1;
    Realtype factor2=0.2;
    
    for (i = 0 ; i < N ; ++i) {
      sum0 = sum0+factor1*factor2;
      sum1 = factor2*factor1 + sum1;
    }
    return sum0-sum1;
  };
  
  Realtype compute_scomdet (void) {
    Realtype sum0=0.;
    Realtype sum1=0.;
    int i;
    
    Realtype factor1=0.1;
    Realtype factor2=0.2;
    for (i = 0 ; i < N ; ++i) {
      sum0 += factor1 *factor2;
      sum1 += ((-factor1) * factor2);
    }
    return sum0+sum1;
  };
  

  Realtype compute_monotonic(void){
    Realtype sum0=1.;
    Realtype sum1=1.;
    
    Realtype inc0= myUnderEpsilon<Realtype>::value;
    Realtype inc1=2* inc0;
    sum0+=inc0;
    sum1+=inc1;
    
    return sum1 -sum0;
  };


};



int main (int argc, char **argv) {
  double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use

  test_compute<float> tFloat;
  test_compute<double> tDouble;

  printf("FLOAT\n");
  tFloat.run();
  printf("\n");
  printf("DOUBLE\n");
  tDouble.run();
  
  return 0;
}
