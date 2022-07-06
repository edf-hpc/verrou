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

  r->current_ = vr_rand_next (r);
  vr_tabulation_hash::genTable((r->gen_));
  //  vr_twisted_tabulation_hash::genTable((r->gen_));
  vr_multiply_shift_hash::genTable((r->gen_));
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



inline double vr_rand_ratio(Vr_Rand *r){
#if VERROU_NUM_AVG==1
  const double res=tinymt64_generate_double(&(r->gen_) );
  return res;
#else
  if(r->count_==loopAvg){
    const uint64_t localGen=tinymt64_generate_uint64(&(r->gen_) );
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


/*
 * produces a pseudo random number in a deterministic way
 * the same seed and inputs will always produce the same output
 */


template<class OP>
inline bool vr_rand_bool_det (const Vr_Rand * r, const typename OP::PackArgs& p) {

#ifdef VERROU_DET_HASH
  typedef VERROU_DET_HASH hash;
  return hash::hashBool(r, p, OP::getHash());
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
}



template<class OP>
inline
const typename OP::RealType
vr_rand_ratio_det (const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_DET_HASH
  typedef VERROU_DET_HASH hash;
  return hash::hashRatio(r, p, OP::getHash());
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
}

template<class OP>
inline bool vr_rand_bool_comdet (const Vr_Rand * r, const typename OP::PackArgs& p) {

#ifdef VERROU_DET_HASH
  typedef VERROU_DET_HASH hash;
  sort_pack<typename OP::PackArgs::RealType,OP::PackArgs::nb> st(p);
  return hash::hashBool(r, st.getPack(), OP::getHash());
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
}



template<class OP>
inline
const typename OP::RealType
vr_rand_ratio_comdet (const Vr_Rand * r, const typename OP::PackArgs& p) {
#ifdef VERROU_DET_HASH
  typedef VERROU_DET_HASH hash;
  //  const typename OP::PackArgs;
  sort_pack<typename OP::PackArgs::RealType,OP::PackArgs::nb> st(p);
  return hash::hashRatio(r, st.getPack(), OP::getHash());
#else
  #error "VERROU_DET_HASH has to be defined"
#endif
}
