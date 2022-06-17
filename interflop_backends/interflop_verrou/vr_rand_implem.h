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

#include "tableHash.hxx"

inline static uint64_t vr_rand_next (Vr_Rand * r){
  return tinymt64_generate_uint64(&(r->gen_) );
}

inline uint32_t vr_loop(){
  return 63;
}

inline int32_t vr_rand_max () {
  int32_t max=2147483647;  //2**31-1
  return max;
}

inline void private_gen_init(Vr_Rand * r){
}




inline void vr_rand_setSeed (Vr_Rand * r, uint64_t c) {
  r->count_   = 0;
  r->seed_    = c;
  private_gen_init(r);
  tinymt64_init(&(r->gen_), r->seed_);
  for(int i=0 ;i < 6; i++  ){
    r->seedTab_[i]=tinymt64_generate_uint64(&(r->gen_));
  }
  r->current_ = vr_rand_next (r);
  tabulationHash::genTable((r->gen_));
  twistedTabulationHash::genTable((r->gen_));
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



#ifndef VERROU_NUM_AVG
#define VERROU_NUM_AVG 2
#endif

#if VERROU_NUM_AVG==8
uint64_t maskAvg = 0x00000000000000FF;  ;
uint64_t shiftAvg= 8 ;
uint32_t loopAvg = 8  ;
double maxAvgInv(1./256.);
#elif VERROU_NUM_AVG==4
uint64_t maskAvg = 0x000000000000FFFF;  ;
uint64_t shiftAvg= 16 ;
uint32_t loopAvg = 4  ;
double maxAvgInv(1/65536.);
#elif VERROU_NUM_AVG==3
uint64_t maskAvg = 0x00000000001FFFFF;// 21bit
uint64_t shiftAvg= 21 ;
uint32_t loopAvg = 3  ;
double maxAvgInv(1/2097152.);
#elif VERROU_NUM_AVG==2
uint64_t maskAvg = 0x00000000FFFFFFFF;  ;
uint64_t shiftAvg= 32 ;
uint32_t loopAvg = 2  ;
double maxAvgInv(1/4294967296.);
#elif VERROU_NUM_AVG==1
#else
#error 'VERROU_NUM_AVG is not defined'
#endif



inline double vr_rand_ratio(Vr_Rand *r){
#if VERROU_NUM_AVG==1
  double res=tinymt64_generate_double(&(r->gen_) );
#else
  if(r->count_==loopAvg){
    r->current_= tinymt64_generate_uint64(&(r->gen_) );
    r->count_=0;
  }
  uint64_t local= r->current_ & maskAvg;
  double res = (double) local *maxAvgInv;
  (r->current_)=((r->current_)>>shiftAvg);
  (r->count_)++;
#endif
  //  double res=vr_rand_next(r)/ vr_rand_max();
  return res;
}


/*
 * produces a pseudo random number in a deterministic way
 * the same seed and inputs will always produce the same output
 */
template<class REALTYPE, int NB>
class dietzfelbingerHash{
public:
  static bool hashBool(const Vr_Rand * r,
		       const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){

    const uint64_t argsHash =  pack.getXorHash();
    const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint64_t oddSeed = seed | 1; // insures seed is odd
    const bool res = (oddSeed * argsHash) >> 63;
    return res;
  }

  static double hashRatio(const Vr_Rand * r,
			  const vr_packArg<REALTYPE,NB>& pack,
			  uint32_t hashOp){

    const uint64_t argsHash =  pack.getXorHash();
    const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
    // returns a one bit hash as a PRNG
    // uses Dietzfelbinger's multiply shift hash function
    // see `High Speed Hashing for Integers and Strings` (https://arxiv.org/abs/1504.06804)
    const uint64_t oddSeed = seed | 1; // insures seed is odd
    const uint32_t res = (oddSeed * argsHash) >> 32;

    return ((double)res / (double)(4294967296) ); //2**32 = 4294967296
  }

};


template<class REALTYPE, int NB>
class multiplyShiftHash{
public:
  static bool hashBool(const Vr_Rand * r,
		       const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){

    const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
    const uint64_t* seedTab=vr_rand_getSeedTab(r);

    const uint64_t m=pack.getMultiply(seedTab);
    return (m+seed)>>63;
  }

  static double hashRatio(const Vr_Rand * r,
		       const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){

    const uint64_t seed = vr_rand_getSeed(r) ^ hashOp;
    const uint64_t* seedTab=vr_rand_getSeedTab(r);

    const uint64_t m=pack.getMultiply(seedTab);
    const uint32_t v=(m+seed)>>32;
    return ((double)v / (double)(4294967296) ); //2**32 = 4294967296
  }

};





#include "mersenneHash.hxx"

template<class OP>
inline bool vr_rand_bool_det (const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_DET_FAST_HASH
  typedef dietzfelbingerHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  return hash::hashBool(r, p, OP::getHash());
#endif
#ifdef VERROU_DET_REF_HASH
  return mersenneHash::hashBool(p, vr_rand_getSeed(r), OP::getHash());
#endif

#if !defined(VERROU_DET_FAST_HASH) && ! defined(VERROU_DET_REF_HASH)
  //  typedef multiplyShiftHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  //  return hash::hashBool(r, p, OP::getHash());
  typedef doubleTabulationHash<twistedTabulationHash> hash;
  return hash::hashBool(p, OP::getHash());
#endif
}


// inline int32_t vr_rand_int (Vr_Rand * r) {
//   uint64_t res=vr_rand_next (r) % vr_rand_max();
//   return (int32_t)res;
// }


template<class OP>
inline
const typename OP::RealType
vr_rand_ratio_det (const Vr_Rand * r, const typename OP::PackArgs& p) {

#ifdef VERROU_DET_FAST_HASH
  typedef dietzfelbingerHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  return (typename OP::RealType)hash::hashRatio(r, p, OP::getHash());
#endif
#ifdef VERROU_DET_REF_HASH
  return (typename OP::RealType)mersenneHash::hashRatio(p, vr_rand_getSeed(r), OP::getHash());
#endif

#if !defined(VERROU_DET_FAST_HASH) && ! defined(VERROU_DET_REF_HASH)
  //  typedef multiplyShiftHash<typename OP::PackArgs::RealType, OP::PackArgs::nb> hash;
  //  return (RealType)hash::hashRatio(r, p, OP::getHash());
  //  typedef tabulationHash hash;
  typedef doubleTabulationHash<twistedTabulationHash> hash;
  return hash::hashRatio(p, OP::getHash());
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
