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

inline uint64_t vr_rand_getSeed (const Vr_Rand * r);


#include "tableHash.hxx"
#include "dietzfelbingerHash.hxx"
#include "multiplyShiftHash.hxx"
#include "mersenneHash.hxx"
#include "xxHash.hxx"


inline static uint64_t vr_rand_next (Vr_Rand * r){
#ifndef USE_XOSHIRO
  return tinymt64_generate_uint64(&(r->gen_) );
#else
  return xoshiro256plus_next(r->rng256_);
#endif
}

inline static uint32_t vr_loop(){
  return 63;
}




inline void vr_rand_setSeed (Vr_Rand * r, uint64_t c) {
  r->count_   = 0;
  r->seed_    = c;

  tinymt64_init(&(r->gen_), r->seed_);
#ifdef USE_XOSHIRO
  init_xoshiro128_state(r->rng128_, r->seed_);
  init_xoshiro256_state(r->rng256_, r->seed_);
#endif
  r->current_ = vr_rand_next (r);
  vr_tabulation_hash::genTable((r->gen_));
  //  vr_twisted_tabulation_hash::genTable((r->gen_));
  vr_multiply_shift_hash::genTable((r->gen_));

  const double p=tinymt64_generate_double(&(r->gen_) );
  r->p=p;
}



inline uint64_t vr_rand_getSeed (const Vr_Rand * r) {
  return r->seed_;
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



//#ifndef VERROU_NUM_AVG
//#define VERROU_NUM_AVG 2
//#endif

#if VERROU_NUM_AVG==8
constexpr uint64_t maskAvg = 0x00000000000000FF;  ;
constexpr uint64_t shiftAvgTab[]= {0, 8 , 16, 24, 32, 40, 48, 56 };
constexpr uint32_t loopAvg = 8  ;
constexpr double maxAvgInv(1./256.);
#elif VERROU_NUM_AVG==4
constexpr uint64_t maskAvg = 0x000000000000FFFF;  ;
constexpr uint64_t shiftAvgTab[]= {0,16, 32, 48};
constexpr uint32_t loopAvg = 4  ;
constexpr double maxAvgInv(1/65536.);
#elif VERROU_NUM_AVG==3
constexpr uint64_t maskAvg = 0x00000000001FFFFF;// 21bit
constexpr uint64_t shiftAvgTab[]={0,21,42} ;
constexpr uint32_t loopAvg = 3  ;
constexpr double maxAvgInv(1/2097152.);
#elif VERROU_NUM_AVG==2
constexpr uint64_t maskAvg = 0x00000000FFFFFFFF;  ;
constexpr uint64_t shiftAvgTab[]={0,32} ;
constexpr uint32_t loopAvg = 2  ;
constexpr double maxAvgInv(1/4294967296.);
#elif VERROU_NUM_AVG==1
#else
#error 'VERROU_NUM_AVG is not defined'
#endif


template<class REALTYPE>
inline REALTYPE vr_rand_ratio(Vr_Rand *r);

template<>
inline double vr_rand_ratio<double>(Vr_Rand *r){
#if VERROU_NUM_AVG==1
#ifndef USE_XOSHIRO
  const double res=tinymt64_generate_double(&(r->gen_) );
#else
  //const double res=xoshiro_uint32_to_float(xoshiro128plus_next(r->rng128_));
  const double res=xoshiro_uint64_to_double(xoroshiro128plus_next(r->rng128_));
#endif
  return res;
#else
  if(r->count_==loopAvg){
#ifndef USE_XOSHIRO
    const uint64_t localGen=tinymt64_generate_uint64(&(r->gen_) );
#else
    const uint64_t localGen=xoshiro256plus_next(r->rng256_);
#endif
    const uint64_t local= localGen & maskAvg;
    const double res = local *maxAvgInv;
    r->count_=1;
    r->current_= localGen;
    return res;
  }
  const uint64_t local= (r->current_ >> (shiftAvgTab[r->count_])) & maskAvg;
  const double res = local *maxAvgInv;
  (r->count_)++;
  return res;
#endif
}


template<>
inline float vr_rand_ratio<float>(Vr_Rand *r){
#if VERROU_NUM_AVG==1
#ifndef USE_XOSHIRO
  const double res=tinymt64_generate_double(&(r->gen_) );
#else
  const float res=xoshiro_uint32_to_float(xoshiro128plus_next(r->rng128_));
  //  const double res=xoshiro_uint64_to_double(xoroshiro128plus_next(r->rng128_));
#endif
  return res;
#else
  if(r->count_==loopAvg){
#ifndef USE_XOSHIRO
    const uint64_t localGen=tinymt64_generate_uint64(&(r->gen_) );
#else
    const uint64_t localGen=xoshiro256plus_next(r->rng256_);
#endif
    const uint32_t local= localGen & maskAvg;
    const float res = local *maxAvgInv;
    r->count_=1;
    r->current_= localGen;
    return res;
  }
  const uint32_t local= (r->current_ >> (shiftAvgTab[r->count_])) & maskAvg;
  const float res = local *maxAvgInv;
  (r->count_)++;
  return res;
#endif
}


template<class OP>
class vr_rand_prng {
public:
  static inline bool
  randBool (Vr_Rand * r, const typename OP::PackArgs& p) {
    return vr_rand_bool(r);
  }

  static inline const typename OP::RealType
  randRatio(Vr_Rand * r, const typename OP::PackArgs& p) {
    return vr_rand_ratio<typename OP::RealType>(r);
  }
};


/*
 * produces a pseudo random number in a deterministic way
 * the same seed and inputs will always produce the same output
 */
template<class OP>
class vr_rand_det {
public:
  static inline bool
  randBool (const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_DET_HASH
    typedef VERROU_DET_HASH hash;
    return hash::hashBool(r, p, OP::getHash());
#else
#error "VERROU_DET_HASH has to be defined"
#endif
  }

  static inline const typename OP::RealType
  randRatio(const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_DET_HASH
    typedef VERROU_DET_HASH hash;
    return hash::hashRatio(r, p, OP::getHash());
#else
#error "VERROU_DET_HASH has to be defined"
#endif
  }
};







class randScomBool{
public:
  typedef bool TypeOut;
#ifdef VERROU_DET_HASH
    typedef VERROU_DET_HASH hashDet;
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
  const Vr_Rand*& r_;
  randScomBool(const Vr_Rand*& r):r_(r){
  }

  template<class REALTYPE, int NB>
  inline TypeOut hash(const vr_packArg<REALTYPE,NB>& p, uint32_t opHash)const{
    return hashDet::hashBool(r_, p, opHash);
  }

  template<class REALTYPE, int NB>
  inline TypeOut hashBar(const vr_packArg<REALTYPE,NB>& p, uint32_t opHash)const{
    return !hashDet::hashBool(r_, p,opHash);
  }
};


class randScomRatio{
public:
  typedef double TypeOut;
#ifdef VERROU_DET_HASH
    typedef VERROU_DET_HASH hashDet;
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
  const Vr_Rand*& r_;
  randScomRatio(const Vr_Rand*& r):r_(r){
  }


  template<class REALTYPE, int NB>
  inline TypeOut hash(const vr_packArg<REALTYPE,NB>& p, uint32_t opHash)const{
    return hashDet::hashRatio(r_, p, opHash);
  }


  template<class REALTYPE, int NB>
  inline TypeOut hashBar(const vr_packArg<REALTYPE,NB>& p, uint32_t opHash)const{
    return 1.- hashDet::hashRatio(r_, p, opHash);
  }
};


/*
 * produces a pseudo random number in a deterministic way
 * the same seed and inputs will always produce the same output
 * if the opertor is commutative the order is not taken into account
 */
template<class OP>
class vr_rand_comdet {
public:

  static inline bool
  randBool(const Vr_Rand * r, const typename OP::PackArgs& p) {
    return OP::hashCom(randScomBool(r),p);
  }


  static inline
  double
  randRatio(const Vr_Rand * r, const typename OP::PackArgs& p) {
    return OP::hashCom(randScomRatio(r),p);
  }
};



template<class OP>
class vr_rand_scomdet {
public:

  static inline bool
  randBool(const Vr_Rand * r, const typename OP::PackArgs& p) {
    return OP::hashScom(randScomBool(r),p);
  }


  static inline
  double
  randRatio(const Vr_Rand * r, const typename OP::PackArgs& p) {
    return OP::hashScom(randScomRatio(r),p);
  }
};



template<class OP, template<class> class RAND>
class vr_rand_p {
public:
  static inline bool
  randBool (Vr_Rand * r, const typename OP::PackArgs& args) {
    return RAND<OP>::randRatio(r,args) < (r->p);
  }
};
