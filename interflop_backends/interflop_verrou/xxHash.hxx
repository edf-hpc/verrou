#pragma once

#if !defined(VGA_arm64)
#define USE_XXH3
#endif

#define USE_SEED_OP

#ifndef USE_XXH3
#include "xxhashct/xxh64.hpp"
#else
#include "xxh3.h"
#endif

#ifndef USE_XOSHIRO
#include "prng/xoshiro.cxx"
#endif

template<class PACKARGS>
struct buffer_hash_op;

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash_op<vr_packArg<REALTYPE,1> >{
  const REALTYPE arg1;
  const uint32_t op;
  buffer_hash_op(const vr_packArg<REALTYPE,1>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    op(hashOp)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash_op<vr_packArg<REALTYPE,2> >{
  const REALTYPE arg1;
  const REALTYPE arg2;
  const uint32_t op;
  buffer_hash_op(const vr_packArg<REALTYPE,2>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    arg2(pack.arg2),
    op(hashOp)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash_op<packargsIntReal<REALTYPE> >{
  const int arg1;
  const REALTYPE arg2;
  const uint32_t op;
  buffer_hash_op(const packargsIntReal<REALTYPE>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    arg2(pack.arg2),
    op(hashOp)
  {
  }
};


template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash_op<vr_packArg<REALTYPE,3> >{
  const REALTYPE arg1;
  const REALTYPE arg2;
  const REALTYPE arg3;
  const uint32_t op;
  buffer_hash_op(const vr_packArg<REALTYPE,3>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    arg2(pack.arg2),
    arg3(pack.arg3),
    op(hashOp)
  {
  }
};


template<class PACKARGS>
struct buffer_hash;

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<vr_packArg<REALTYPE,1> >{
  const REALTYPE arg1;
  buffer_hash(const vr_packArg<REALTYPE,1>& pack):
    arg1(pack.arg1)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<vr_packArg<REALTYPE,2> >{
  const REALTYPE arg1;
  const REALTYPE arg2;
  buffer_hash(const vr_packArg<REALTYPE,2>& pack):
    arg1(pack.arg1),
    arg2(pack.arg2)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<packargsIntReal<REALTYPE> >{
  const int arg1;
  const REALTYPE arg2;
  buffer_hash(const packargsIntReal<REALTYPE>& pack):
    arg1(pack.arg1),
    arg2(pack.arg2)
  {
  }
};



template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<vr_packArg<REALTYPE,3> >{
  const REALTYPE arg1;
  const REALTYPE arg2;
  const REALTYPE arg3;
  buffer_hash(const vr_packArg<REALTYPE,3>& pack):
    arg1(pack.arg1),
    arg2(pack.arg2),
    arg3(pack.arg3)
  {
  }
};

class vr_xxhash_hash{
public:
  typedef vr_xxhash_hash xxHash;

  static inline void genTable(tinymt64_t& gen){
#ifdef USE_XXH3
    const uint64_t seed=tinymt64_generate_uint64(&gen);
    update_pre_computed_bitflip(seed);
#endif
  };

  template<class PACKARGS>
  static inline bool hashBool(const Vr_Rand * r,
			      const PACKARGS& pack,
			      uint32_t hashOp){
    const uint64_t seed=vr_rand_getSeed(r);
#ifndef USE_XXH3
    const buffer_hash_op<PACKARGS> buffer(pack,hashOp);
    const uint64_t hashValue = xxh64::hash((const char*)&buffer, sizeof(buffer_hash_op<PACKARGS>), seed);
#else
#ifdef USE_SEED_OP
    const buffer_hash<PACKARGS> buffer(pack);
    const uint64_t hashValue =  XXH3_64bits_withSeed((const char*)&buffer, sizeof(buffer_hash<PACKARGS>), seed ^ hashOp);
#else
    const buffer_hash_op<PACKARGS> buffer(pack,hashOp);
    const uint64_t hashValue =  XXH3_64bits_withSeed((const char*)&buffer, sizeof(buffer_hash_op<PACKARGS>), seed);
#endif
#endif

    return (hashValue>>63);
  };

  template<class PACKARGS>
  static inline double hashRatio(const Vr_Rand * r,
				 const PACKARGS& pack,
				 uint32_t hashOp){
    const uint64_t seed=vr_rand_getSeed(r);

#ifndef USE_XXH3
    const buffer_hash_op<PACKARGS> buffer(pack,hashOp);
    const uint64_t hashValue = xxh64::hash((const char*)&buffer, sizeof(buffer_hash_op<PACKARGS>), seed);
#else
#ifdef USE_SEED_OP
    const buffer_hash<PACKARGS> buffer(pack);
    const uint64_t hashValue =  XXH3_64bits_withSeed((const char*)&buffer, sizeof(buffer_hash<PACKARGS>), seed ^ hashOp);
#else
    const buffer_hash_op<PACKARGS> buffer(pack,hashOp);
    const uint64_t hashValue = XXH3_64bits_withSeed( (const char*)&buffer, sizeof(buffer_hash_op<PACKARGS>), seed);
#endif
#endif
    return xoshiro_uint64_to_double(hashValue);
  };


  static inline double hashRatioFromResult(const Vr_Rand * r,
					   const double* res){

#ifndef USE_XXH3
    const uint64_t seed=vr_rand_getSeed(r);
    const uint64_t hashValue = xxh64::hash((const char*)res, sizeof(double), seed);
#else
    const uint64_t hashValue=XXH3_64bits_withSeed_with_precomputedbitflip(res);
    //    const uint64_t hashValue =  XXH3_64bits_withSeed((const char*)res, sizeof(REALTYPE), seed);
#endif
    return xoshiro_uint64_to_double(hashValue);
  };

  static inline double hashRatioFromResult(const Vr_Rand * r,
					   const float* res){
#ifndef USE_XXH3
    const uint64_t seed=vr_rand_getSeed(r);
    const uint64_t hashValue = xxh64::hash((const char*)res, sizeof(float), seed);
#else
    //    const uint64_t hashValue =  XXH3_64bits_withSeed((const char*)res, sizeof(REALTYPE), seed);
    const uint64_t hashValue=XXH3_64bits_withSeed_with_precomputedbitflip(res);
#endif
    return xoshiro_uint64_to_double(hashValue);
  };

};
