#pragma once

#include "xxhashct/xxh64.hpp"

#ifndef USE_XOSHIRO
#include "prng/xoshiro.cxx"
#endif

template<class REALTYPE, int NB>
struct buffer_hash;

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<REALTYPE,1>{
  REALTYPE arg1;
  uint32_t op;
  buffer_hash(const vr_packArg<REALTYPE,1>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    op(hashOp)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<REALTYPE,2>{
  REALTYPE arg1;
  REALTYPE arg2;
  uint32_t op;
  buffer_hash(const vr_packArg<REALTYPE,2>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    arg2(pack.arg2),
    op(hashOp)
  {
  }
};

template<class REALTYPE>
struct __attribute__ ((__packed__)) buffer_hash<REALTYPE,3>{
  REALTYPE arg1;
  REALTYPE arg2;
  REALTYPE arg3;
  uint32_t op;
  buffer_hash(const vr_packArg<REALTYPE,3>& pack, uint32_t hashOp):
    arg1(pack.arg1),
    arg2(pack.arg2),
    arg3(pack.arg3),
    op(hashOp)
  {
  }
};

class vr_xxhash_hash{
public:
  typedef vr_xxhash_hash xxHash;

  template<class REALTYPE, int NB>
  static inline bool hashBool(const Vr_Rand * r,
			      const vr_packArg<REALTYPE,NB>& pack,
			      uint32_t hashOp){
    const uint64_t seed=vr_rand_getSeed(r);
    const buffer_hash<REALTYPE,NB> buffer(pack,hashOp);
    uint64_t hashValue = xxh64::hash((char*)&buffer, sizeof(buffer_hash<REALTYPE,NB>), seed);
    return (hashValue>>63);
  };

  template<class REALTYPE, int NB>
  static inline double hashRatio(const Vr_Rand * r,
				 const vr_packArg<REALTYPE,NB>& pack,
				 uint32_t hashOp){
    const uint64_t seed=vr_rand_getSeed(r);
    const buffer_hash<REALTYPE,NB> buffer(pack,hashOp);
    const uint64_t hashValue = xxh64::hash((char*)&buffer, sizeof(buffer_hash<REALTYPE,NB>), seed);
    return xoshiro_uint64_to_double(hashValue);
  };

};
