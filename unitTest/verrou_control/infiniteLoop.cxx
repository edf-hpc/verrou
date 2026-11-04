#include <iostream>


double infLoop(double x){
  while( x >0 )
   {
      x = x / 1.1;
   }
  return x;
}

#define NBTASKLOOP 1

int main(int argc, char** argv){
  double x[NBTASKLOOP];
  for(int i=0; i<10; i++){
    x[i]=100000+i;
  }
  //#pragma omp parallel for
  for(int i=0; i<NBTASKLOOP; i++){
    x[i]=infLoop(infLoop(x[i]));
  };
  for(int i=0; i<NBTASKLOOP; i++){
    std::cout << x[i]<<std::endl;
  }
  return 0;
}
