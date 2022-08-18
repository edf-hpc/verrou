
#include<iostream>

#include "xoshiro.cxx"
int main(int argc, char** argv){

  uint64_t seed=42;
  xoshiro256_state_t state256;
  init_xoshiro256_state(state256, seed);

  std::cout <<std::endl<<"xoshiro256starstar"<<std::endl;
  std::cout << xoshiro256starstar_next(state256)<<std::endl;
  std::cout << xoshiro256starstar_next(state256)<<std::endl;
  std::cout << xoshiro256starstar_next(state256)<<std::endl;
  std::cout << xoshiro256starstar_next(state256)<<std::endl;
  std::cout << xoshiro256starstar_next(state256)<<std::endl;

  std::cout <<std::endl<<"xoshiro256plusplus"<<std::endl;
  init_xoshiro256_state(state256, seed);
  std::cout << xoshiro256plusplus_next(state256)<<std::endl;
  std::cout << xoshiro256plusplus_next(state256)<<std::endl;
  std::cout << xoshiro256plusplus_next(state256)<<std::endl;
  std::cout << xoshiro256plusplus_next(state256)<<std::endl;
  std::cout << xoshiro256plusplus_next(state256)<<std::endl;

  std::cout <<std::endl<<"xoshiro256plus"<<std::endl;
  init_xoshiro256_state(state256, seed);
  std::cout << xoshiro256plus_next(state256)<<std::endl;
  std::cout << xoshiro256plus_next(state256)<<std::endl;
  std::cout << xoshiro256plus_next(state256)<<std::endl;
  std::cout << xoshiro256plus_next(state256)<<std::endl;
  std::cout << xoshiro256plus_next(state256)<<std::endl;


  xoshiro128_state_t state128;
  init_xoshiro128_state(state128, seed);

  std::cout <<std::endl<<"xoshiro128starstar"<<std::endl;
  std::cout << xoshiro128starstar_next(state128)<<std::endl;
  std::cout << xoshiro128starstar_next(state128)<<std::endl;
  std::cout << xoshiro128starstar_next(state128)<<std::endl;
  std::cout << xoshiro128starstar_next(state128)<<std::endl;
  std::cout << xoshiro128starstar_next(state128)<<std::endl;

  std::cout <<std::endl<<"xoshiro128plusplus"<<std::endl;
  init_xoshiro128_state(state128, seed);
  std::cout << xoshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoshiro128plusplus_next(state128)<<std::endl;

  std::cout <<std::endl<<"xoshiro128plus"<<std::endl;
  init_xoshiro128_state(state128, seed);
  std::cout << xoshiro128plus_next(state128)<<std::endl;
  std::cout << xoshiro128plus_next(state128)<<std::endl;
  std::cout << xoshiro128plus_next(state128)<<std::endl;
  std::cout << xoshiro128plus_next(state128)<<std::endl;
  std::cout << xoshiro128plus_next(state128)<<std::endl;



  std::cout <<std::endl<<"xoroshiro128starstar"<<std::endl;
  init_xoshiro128_state(state128, seed);
  std::cout << xoroshiro128starstar_next(state128)<<std::endl;
  std::cout << xoroshiro128starstar_next(state128)<<std::endl;
  std::cout << xoroshiro128starstar_next(state128)<<std::endl;
  std::cout << xoroshiro128starstar_next(state128)<<std::endl;
  std::cout << xoroshiro128starstar_next(state128)<<std::endl;

  std::cout <<std::endl<<"xoroshiro128plusplus"<<std::endl;
  init_xoshiro128_state(state128, seed);
  std::cout << xoroshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoroshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoroshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoroshiro128plusplus_next(state128)<<std::endl;
  std::cout << xoroshiro128plusplus_next(state128)<<std::endl;

  std::cout <<std::endl<<"xoroshiro128plus"<<std::endl;
  init_xoshiro128_state(state128, seed);
  std::cout << xoroshiro128plus_next(state128)<<std::endl;
  std::cout << xoroshiro128plus_next(state128)<<std::endl;
  std::cout << xoroshiro128plus_next(state128)<<std::endl;
  std::cout << xoroshiro128plus_next(state128)<<std::endl;
  std::cout << xoroshiro128plus_next(state128)<<std::endl;




  std::cout << std::endl <<"conv"<<std::endl;

  for(int i=0; i<5; i++){
    std::cout << xoshiro_uint64_to_double( xoroshiro128plus_next(state128))<<std::endl;
    std::cout << xoshiro_uint32_to_float( xoshiro128plus_next(state128))<<std::endl;
  };

}
