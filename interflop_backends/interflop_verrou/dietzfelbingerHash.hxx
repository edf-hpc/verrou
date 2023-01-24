#pragma once

class vr_dietzfelbinger_hash{
public:
  template<int NB>
  static inline bool hashBool(const Vr_Rand * r,
			      const vr_packArg<double,NB>& pack,
			      uint32_t hashOp){

    const uint64_t argsHash =  vr_dietzfelbinger_hash::xorHash(pack);
    const uint64_t seed = vr_rand_getSeed(r) ^ (hashOp<<1); //<<1 to avoid conflict with |1
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint64_t oddSeed = seed | 1; // insures seed is odd
    const bool res = (oddSeed * argsHash) >> 63;
    return res;
  }

  template<int NB>
  static inline bool hashBool(const Vr_Rand * r,
			      const vr_packArg<float,NB>& pack,
			      uint32_t hashOp){

    const uint32_t argsHash =  vr_dietzfelbinger_hash::xorHash(pack);
    const uint32_t seed = vr_rand_getSeed(r) ^ (hashOp<<1); //<<1 to avoid conflict with |1
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint32_t oddSeed = seed | 1; // insures seed is odd
    const bool res = (oddSeed * argsHash) >> 31;
    return res;
  }

  


  template<int NB>
  static inline double hashRatio(const Vr_Rand * r,
			  const vr_packArg<double,NB>& pack,
			  uint32_t hashOp){

    const uint64_t argsHash =  vr_dietzfelbinger_hash::xorHash(pack);
    const uint64_t seed = vr_rand_getSeed(r) ^ (hashOp<<1);
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint64_t oddSeed = seed | 1; // insures seed is odd
    const uint32_t res = (oddSeed * argsHash) >> 32;
    constexpr double invMAx=(1/ 4294967296.);

    return ((double)res * invMAx ); //2**32 = 4294967296
  }
  
  template<int NB>
  static inline double hashRatio(const Vr_Rand * r,
			  const vr_packArg<float,NB>& pack,
			  uint32_t hashOp){

    const uint32_t argsHash =  vr_dietzfelbinger_hash::xorHash(pack);;
    const uint32_t seed = vr_rand_getSeed(r) ^ (hashOp<<1);
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint32_t oddSeed = seed | 1; // insures seed is odd
    const uint32_t res = (oddSeed * argsHash);
    constexpr double invMAx=(1/ 4294967296.);

    return ((double)res * invMAx ); //2**32 = 4294967296
  }


  
  static inline uint64_t xorHash(const vr_packArg<double,1>& pack){
      return realToUint64_reinterpret_cast<double>(pack.arg1);
  };
  static inline uint32_t xorHash(const vr_packArg<float,1>& pack){
      return realToUint32_reinterpret_cast(pack.arg1);
  };
  static inline uint64_t xorHash(const vr_packArg<double,2>& pack){
      return realToUint64_reinterpret_cast<double>(pack.arg1) ^ realToUint64_reinterpret_cast<double>(pack.arg2);
  };
  static inline uint32_t xorHash(const vr_packArg<float,2>& pack){
    return realToUint32_reinterpret_cast(pack.arg1)^ realToUint32_reinterpret_cast(pack.arg2);
  };

  static inline uint64_t xorHash(const vr_packArg<double,3>& pack){
    return realToUint64_reinterpret_cast<double>(pack.arg1)
      ^ realToUint64_reinterpret_cast<double>(pack.arg2)
      ^ realToUint64_reinterpret_cast<double>(pack.arg3);
  };
  static inline uint32_t xorHash(const vr_packArg<float,3>& pack){
    return realToUint32_reinterpret_cast(pack.arg1)
      ^ realToUint32_reinterpret_cast(pack.arg2)
      ^ realToUint32_reinterpret_cast(pack.arg3);
  };


};
