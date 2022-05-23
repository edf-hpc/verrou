/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for random number generation.                      ---*/
/*---                                             vr_rand_implem.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU Lesser General Public License is contained in the file COPYING.
*/


//#include "vr_rand.h"

//Warning FILE include in vr_rand.h

#include "vr_op.hxx"

#ifdef VERROU_FAST_GEN
inline static uint64_t vr_rand_next (Vr_Rand * r){
  r->next_ = r->next_ * 1103515245 + 12345;
  return (uint64_t)((r->next_/65536) % 32768);
}
inline int32_t vr_rand_max () {
  return 32767;
}

inline int32_t vr_loop(){
  return 14; // 2**15= 32768
}

inline void private_gen_init(Vr_Rand * r){
    r->next_    = r->seed_;
}

#else
inline static uint64_t vr_rand_next (Vr_Rand * r){
  return tinymt64_generate_uint64(&(r->gen_) );
}
inline int32_t vr_rand_max () {
  int32_t max=2147483647;  //2**21-1
  return max;
}

inline int32_t vr_loop(){
  return 63; // 2**15= 32768
}

inline void private_gen_init(Vr_Rand * r){
}
#endif


inline void vr_rand_setSeed (Vr_Rand * r, uint64_t c) {
  r->count_   = 0;
  r->seed_    = c;
  private_gen_init(r);
  tinymt64_init(&(r->gen_), r->seed_);
  for(int i=0 ;i < 6; i++  ){
    r->seedTab_[i]=tinymt64_generate_uint64(&(r->gen_));
  }
  r->current_ = vr_rand_next (r);
}



inline uint64_t vr_rand_getSeed (const Vr_Rand * r) {
  return r->seed_;
}

inline const uint64_t* vr_rand_getSeedTab (const Vr_Rand * r) {
  return r->seedTab_;
}


inline bool vr_rand_bool (Vr_Rand * r) {
  if (r->count_ == vr_loop()){
    r->current_ = vr_rand_next (r);
    r->count_ = 0;
  }
  bool res = (r->current_ >> (r->count_++)) & 1;
  // VG_(umsg)("Count : %u  res: %u\n", r->count_ ,res);
  return res;
}

/*
 * produces a pseudo random number in a deterministic way
 * the same seed and inputs will always produce the same output
 */
template<class REALTYPE, int NB, class OP>
class dietzfelbingerHash{
public:
  static bool apply(const Vr_Rand * r,
		    const vr_packArg<REALTYPE,NB>& pack,
		    uint32_t hashOp){

    const uint64_t argsHash =  pack.getXorHash();
    const uint64_t seed = vr_rand_getSeed(r) ^ OP::getHash();
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint64_t oddSeed = seed | 1; // insures seed is odd
    const bool res = (oddSeed * argsHash) >> 63;
    return res;
  }
};


template<class REALTYPE, int NB>
class tableHash{
public:
  static bool apply(const Vr_Rand * r,
		    const vr_packArg<REALTYPE,NB>& pack,
		    uint32_t hashOp){

    const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
    const uint64_t* seedTab=vr_rand_getSeedTab(r);

    const uint64_t m=pack.getMultiply(seedTab);
    return (m+seed)>>63;
  }
};




template<class REALTYPE, int NB>
class mersenneHash{
public:
  static bool apply(const Vr_Rand * r,
		    const vr_packArg<REALTYPE,NB>& pack,
		    uint32_t hashOp){
      const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
      const uint64_t res64=pack.getMersenneHash(seed);
      return res64>>63;
  }
};

template<class OP>
inline bool vr_rand_bool_det (const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_FAST_HASH
  typedef dietzfelbingerHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  return hash::apply(r, p, OP::getHash());
#endif
#ifdef VERROU_PRECISE_HASH
  typedef mersenneHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  return hash::apply(r, p, OP::getHash());
#endif

#if !defined(VERROU_PRECISE_HASH) && ! defined(VERROU_PRECISE_HASH)
  typedef tableHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  return hash::apply(r, p, OP::getHash());
#endif
  /*
  const uint64_t argsHash = OP::getHash() ^ p.getHash();
  const uint64_t seed = vr_rand_getSeed(r);
  // returns a one bit hash as a PRNG
  // uses Dietzfelbinger's multiply shift hash function
  // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
  const uint64_t oddSeed = seed | 1; // insures seed is odd
  const bool res = (oddSeed * argsHash) >> 63;
  return res;
*/
}



inline int32_t vr_rand_int (Vr_Rand * r) {
  uint64_t res=vr_rand_next (r) % vr_rand_max();
  return (int32_t)res;
}
