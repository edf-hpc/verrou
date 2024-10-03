/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Implementation of the software implementation of rounding    ---*/
/*--- mode switching.                                              ---*/
/*---                                            vr_roundingOp.hxx ---*/
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


#pragma once
#include <limits>
#include <math.h> //pour isinf
//#ifndef LIBMATHINTERP
extern vr_RoundingMode ROUNDINGMODE;
//#else
//extern vr_RoundingMode ROUNDINGMODE;
//#endif

#ifdef PROFILING_EXACT
extern unsigned int vr_NumOp;
extern unsigned int vr_NumExactOp;
#define INC_OP {vr_NumOp++;}
#define INC_EXACTOP {vr_NumExactOp++;}
#else
#define INC_OP
#define INC_EXACTOP
#endif

//#include "vr_fpRepr.hxx"
#include "vr_nextUlp.hxx"
#include "vr_isNan.hxx"

#include "vr_op.hxx"

template<class OP>
class RoundingNearest{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    return res;
  } ;

};



template<class OP>
class RoundingFloat{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    vr_storeFloat<PackArgs> roundedArgs (p);
    const float res=OP::FloatOp::nearestOp(roundedArgs.getPack()) ;
    return RealType(res);
  } ;

};


template<class REALTYPE>
struct nextTool{
  typedef REALTYPE RealType;

  static inline RealType next_safe(RealType res, RealType signError){
    //assert(signError!=0)
    if(res>0){
      if(signError>0){
	return nextAwayFromZero<RealType>(res);
      }else{
	return nextTowardZero<RealType>(res);
      }
    }
    if(res<0){
      if(signError<0){
	return nextAwayFromZero<RealType>(res);
      }else{
	return nextTowardZero<RealType>(res);
      }
    }
    //if(res==0){
    if((signError)>0){
      return std::numeric_limits<RealType>::denorm_min();
    }else{
      return -std::numeric_limits<RealType>::denorm_min();
    }
  }

  static inline RealType next_unsafe(RealType res, RealType signError){
    //assert(res!=0 && signError!=0)
    if(res>0){
      if(signError>0){
	return nextAwayFromZero<RealType>(res);
      }else{
	return nextTowardZero<RealType>(res);
      }
    }
    if(signError<0){
      return nextAwayFromZero<RealType>(res);
    }else{
      return nextTowardZero<RealType>(res);
    }
  }


  static inline RealType nextAway_safe(RealType res, RealType signError){
    if( (signError>0 && res >0)||(signError<0 && res<0) ){
      return  nextAwayFromZero<RealType>(res);
    }
    if(res==0.){
      if(signError>0){
	return std::numeric_limits<RealType>::denorm_min();
      }
      if(signError<0){
	return -std::numeric_limits<RealType>::denorm_min();
      }
    }
    return res;
  }

  static inline RealType nextAway_unsafe(RealType res, RealType signError){
    if( (signError>0 && res >0)||(signError<0 && res<0) ){
      return  nextAwayFromZero<RealType>(res);
    }
    return res;
  }


  static inline RealType nextAfter_safe(RealType res){
    if(res >0){
      return nextAwayFromZero<RealType>(res);
    }
    if(res < 0){
      return nextTowardZero<RealType>(res);
    }
    return std::numeric_limits<RealType>::denorm_min();
  }

  static inline RealType nextPrev_safe(RealType res){
    if(res <0){
      return nextAwayFromZero<RealType>(res);
    }
    if(res > 0){
      return nextTowardZero<RealType>(res);
    }
    return -std::numeric_limits<RealType>::denorm_min();
  }

  static inline RealType nextAfter_unsafe(RealType res){
    if(res >0){
      return nextAwayFromZero<RealType>(res);
    }
    return nextTowardZero<RealType>(res);
  }

  static inline RealType nextPrev_unsafe(RealType res){
    if(res <0){
      return nextAwayFromZero<RealType>(res);
    }
    return nextTowardZero<RealType>(res);
  }
};

template<class REALTYPE, bool SIGN_DENORM_HACK_NEEDED>
struct nextForRandom;


template<class REALTYPE>
struct nextForRandom<REALTYPE,false>{
  typedef REALTYPE RealType;

  static inline RealType next(const RealType& res, const RealType& sign){
    return nextTool<RealType>::next_unsafe(res,sign);
  }
};


template<>
struct nextForRandom<double,true>{
  typedef double RealType;

  static inline RealType next(const RealType& res, const RealType& sign){
#ifdef VERROU_DENORM_HACKS_DOUBLE
    return nextTool<RealType>::next_safe(res,sign);
#else
    return nextTool<RealType>::next_unsafe(res,sign);
#endif
  }
};


template<>
struct nextForRandom<float,true>{
  typedef float RealType;

  static inline RealType next(const RealType& res, const RealType& sign){
#ifdef VERROU_DENORM_HACKS_FLOAT
    return nextTool<RealType>::next_safe(res,sign);
#else
    return nextTool<RealType>::next_unsafe(res,sign);
#endif
  }
};




template<class OP, class RAND>
class RoundingRandom{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p ){
    const RealType res=OP::nearestOp(p);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType signError=OP::sameSignOfError(p,res);


    if(signError==0.){
      INC_EXACTOP;
      return res;
    }else{
      const bool doNoChange = RAND::randBool(&vr_rand,p);
      if(doNoChange){
	return res;
      }else{
	return nextForRandom<RealType, OP::sign_denorm_hack_needed >::next(res,signError);
      }
    }
  } ;
};




template<class REALTYPE, bool SIGN_DENORM_HACK_NEEDED>
struct nextForPRandom;


template<class REALTYPE>
struct nextForPRandom<REALTYPE,false>{
  typedef REALTYPE RealType;

  static inline RealType nextAfter(const RealType& res){
    return nextTool<RealType>::nextAfter_unsafe(res);
  }
  static inline RealType nextPrev(const RealType& res){
    return nextTool<RealType>::nextPrev_unsafe(res);
  }
};


template<>
struct nextForPRandom<double,true>{
  typedef double RealType;

  static inline RealType nextAfter(const RealType& res){
#ifdef VERROU_DENORM_HACKS_DOUBLE
    return nextTool<RealType>::nextAfter_safe(res);
#else
    return nextTool<RealType>::nextAfter_unsafe(res);
#endif
  }

    static inline RealType nextPrev(const RealType& res){
#ifdef VERROU_DENORM_HACKS_DOUBLE
    return nextTool<RealType>::nextPrev_safe(res);
#else
    return nextTool<RealType>::nextPrev_unsafe(res);
#endif
  }
};

template<>
struct nextForPRandom<float,true>{
  typedef float RealType;

  static inline RealType nextAfter(const RealType& res){
#ifdef VERROU_DENORM_HACKS_FLOAT
    return nextTool<RealType>::nextAfter_safe(res);
#else
    return nextTool<RealType>::nextAfter_unsafe(res);
#endif
  }

    static inline RealType nextPrev(const RealType& res){
#ifdef VERROU_DENORM_HACKS_FLOAT
    return nextTool<RealType>::nextPrev_safe(res);
#else
    return nextTool<RealType>::nextPrev_unsafe(res);
#endif
  }
};




template<class OP, class RAND>
class RoundingPRandom{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p ){
    const RealType res=OP::nearestOp(p);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType signError=OP::sameSignOfError(p,res);

    if(signError==0.){
      INC_EXACTOP;
      return res;
    }else{
      if( signError > 0){
	const bool doNoChange = RAND::randBool(&vr_rand, p);
	if(doNoChange){
	  return res;
	}else{
	  return nextForPRandom<RealType,OP::sign_denorm_hack_needed>::nextAfter(res);
	}
      }
      const bool doNoChange = !RAND::randBool(&vr_rand, p);
      if(doNoChange){
	return res;
      }else{
	return nextForPRandom<RealType,OP::sign_denorm_hack_needed>::nextPrev(res);
      }
    }
  } ;
};



template<class OP, class RAND>
class RoundingSRSMonotonic{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p ){
    const RealType res=OP::nearestOp(p);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType error=OP::error(p,res);

    if(error==0.){
      INC_EXACTOP;
      return res;
    }else{
      bool down;
      RealType resm,resp;
      if( error > 0){
	resm=res;
	if(res >0){
	  resp=nextAwayFromZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  down =( error < (limit * u));
	}else{//res <0 : res==0 => error=>0
	  resp=nextTowardZero<RealType>(res);
	  const RealType resHash(-resp);
	  const RealType limit=1.-RAND::randRatioFromResult(&vr_rand, &resHash);
	  const RealType u(resp-resm);
	  down =( error <= (limit * u));
	}

      }else{
	resp=res;
	if(res >0){
	  resm=nextTowardZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  const RealType errorTh(u+error);
	  down =( errorTh < (limit * u));
	}else{
	  resm=nextAwayFromZero<RealType>(res);
	  const RealType resHash(-resp);
	  //	  const RealType limit=1.-RAND::randRatioFromResult(&vr_rand, &resHash);
	  const RealType x=RAND::randRatioFromResult(&vr_rand, &resHash);
	  const RealType u(resp-resm);
	  //	  const RealType errorTh(u+error);
	  down =( error <= (-x) * u);
	}

      }
      if(down){
	return resm;
      }else{
	return resp;
      }
    }
  };
};



template<class OP, class RAND>
class RoundingSRMonotonic{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p ){
    const RealType res=OP::nearestOp(p);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType error=OP::error(p,res);

    if(error==0.){
      INC_EXACTOP;
      return res;
    }else{
      bool down;
      RealType resm,resp;
      if( error > 0){
	resm=res;
	if(res >0){
	  resp=nextAwayFromZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  down =( error < (limit * u));
	}else{//res <0 : res==0 => error=>0
	  resp=nextTowardZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  down =( error <= (limit * u));
	}

      }else{
	resp=res;
	if(res >0){
	  resm=nextTowardZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  const RealType errorTh(u+error);
	  down =( errorTh < (limit * u));
	}else{
	  resm=nextAwayFromZero<RealType>(res);
	  const RealType limit=RAND::randRatioFromResult(&vr_rand, &resm);
	  const RealType u(resp-resm);
	  const RealType errorTh(u+error);
	  down =( errorTh <= (limit * u));
	}

      }
      if(down){
	return resm;
      }else{
	return resp;
      }
    }
  };
};




template<class OP, class RAND>
class RoundingAverage{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;

    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType error=OP::error(p,res);
    if(error==0.){
      INC_EXACTOP;
      return res;
    }


    if(error>0){
      const RealType nextRes(nextAfter<RealType>(res));
      const RealType u(nextRes -res);
      const bool doNotChange = ((RAND::randRatio(&vr_rand, p) * u)
				>   error);
      if(doNotChange){
	return res;
      }else{
	return nextRes;
      }

    }
    //    if(error<0)
    {
      const RealType prevRes(nextPrev<RealType>(res));
      const RealType mu(prevRes -res);
      const bool doNotChange = ((RAND::randRatio(&vr_rand, p) * mu)
				< error);
      if(doNotChange){
	return res;
      }else{
	return prevRes;
      }
    }
    //return res; //Should not occur
  } ;
};

template<class OP, class RAND>
class RoundingSAverage{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;

    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType error=OP::error(p,res);
    if(error==0.){
      INC_EXACTOP;
      return res;
    }


    if(error>0){
      const RealType nextRes(nextAfter<RealType>(res));
      const RealType u(nextRes -res);
      const bool doNotChange = (((1.-RAND::randRatio(&vr_rand, p)) * u)
				>   error);
      if(doNotChange){
	return res;
      }else{
	return nextRes;
      }

    }
    //    if(error<0)
    {
      const RealType prevRes(nextPrev<RealType>(res));
      const RealType mu(prevRes -res);
      const bool doNotChange = ((RAND::randRatio(&vr_rand, p) * mu)
				< error);
      if(doNotChange){
	return res;
      }else{
	return prevRes;
      }
    }
    //return res; //Should not occur
  } ;
};


template<class OP>
class RoundingZero{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    INC_OP;
    OP::check(p,res);

#ifndef VERROU_IGNORE_NANINF_CHECK
    if(isNanInf<RealType>(res)){
      if( (res!=std::numeric_limits<RealType>::infinity()) && (res!=-std::numeric_limits<RealType>::infinity())  ){
	return res;
      }else{
	if(OP::isInfNotSpecificToNearest(p)){
	  return res;
	}else{
	  if(res>0){
	    return std::numeric_limits<RealType>::max();
	  }else{
	    return -std::numeric_limits<RealType>::max();
	  }
	  }
      }
    }
#endif
    const RealType signError=OP::sameSignOfError(p,res);
#ifdef PROFILING_EXACT
    if(signError==0.){
      INC_EXACTOP;
    }
#endif

    if( (signError>0 && res <0)||(signError<0 && res>0) ){
      return nextTowardZero<RealType>(res);
    }
    return res;
  } ;
};




template<class REALTYPE, bool SIGN_DENORM_HACK_NEEDED>
struct nextForAway;


template<class REALTYPE>
struct nextForAway<REALTYPE,false>{
  typedef REALTYPE RealType;

  static inline RealType nextAway(const RealType& res, const RealType& signError){
    return nextTool<RealType>::nextAway_unsafe(res,signError);
  }
};


template<>
struct nextForAway<double,true>{
  typedef double RealType;

  static inline RealType nextAway(const RealType& res, const RealType& signError){
#ifdef VERROU_DENORM_HACKS_DOUBLE
    return nextTool<RealType>::nextAway_safe(res,signError);
#else
    return nextTool<RealType>::nextAway_unsafe(res,signError);
#endif
  }
};


template<>
struct nextForAway<float,true>{
  typedef float RealType;

  static inline RealType nextAway(const RealType& res, const RealType& signError){
#ifdef VERROU_DENORM_HACKS_FLOAT
    return nextTool<RealType>::nextAway_safe(res,signError);
#else
    return nextTool<RealType>::nextAway_unsafe(res,signError);
#endif
  }
};

template<class OP>
class RoundingAwayZero{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    INC_OP;
    OP::check(p,res);
#ifndef VERROU_IGNORE_NANINF_CHECK
    if(isNanInf<RealType>(res)){
      return res;
    }
#endif
    const RealType signError=OP::sameSignOfError(p,res);
#ifdef PROFILING_EXACT
    if(signError==0.){
      INC_EXACTOP;
    }
#endif
    return nextForAway<RealType, OP::sign_denorm_hack_needed >::nextAway(res,signError);
  } ;
};




template<class OP>
class RoundingUpward{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if(isNanInf<RealType>(res)){
      if(res!=-std::numeric_limits<RealType>::infinity()){
	return res;
      }else{
	if(OP::isInfNotSpecificToNearest(p)){
	  return res;
	}else{
	  return -std::numeric_limits<RealType>::max();
	}
      }
    }
#endif
    const RealType signError=OP::sameSignOfError(p,res);
#ifdef PROFILING_EXACT
    if(signError==0.){
      INC_EXACTOP;
    }
#endif

    if(signError>0.){
      if(res==0.){
	return std::numeric_limits<RealType>::denorm_min();
      }
      if(res==-std::numeric_limits<RealType>::denorm_min()){
	return 0.;
      }
       return nextAfter<RealType>(res);
    }
    return res;
  } ;
};


template<class OP>
class RoundingDownward{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if(isNanInf<RealType>(res)){
      if(res!=std::numeric_limits<RealType>::infinity()){
	return res;
      }else{
	if(OP::isInfNotSpecificToNearest(p)){
	  return res;
	}else{
	  return std::numeric_limits<RealType>::max();
	}
      }
    }
#endif

    const RealType signError=OP::sameSignOfError(p,res);
#ifdef PROFILING_EXACT
    if(signError==0){
      INC_EXACTOP;
    }
#endif
    if(signError<0){
      if(res==0.){
	return -std::numeric_limits<RealType>::denorm_min();
      }
      if(res==std::numeric_limits<RealType>::denorm_min()){
	return 0.;
      }
      return nextPrev<RealType>(res);
    }
    return res;
  } ;
};



template<class OP>
class RoundingFarthest{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    INC_OP;
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf<RealType> (res)){
      return res;
    }
#endif
    OP::check(p,res);
    const RealType error=OP::error(p,res);
    if(error==0.){
      INC_EXACTOP;
      return res;
    }
    if(error>0){
      const RealType newRes=nextAfter<RealType>(res);
      const RealType ulp(newRes-res);
      if(2*error < ulp ){
	return newRes;
      }else{
	return res;
      }
    }else{//error<0
      const RealType newRes=nextPrev<RealType>(res);
      const RealType ulp(res-newRes);
      if(-2*error < ulp ){
	return newRes;
      }else{
	return res;
      }
    }
  }
};




#include "vr_op.hxx"

template<class OP>
class OpWithDynSelectedRoundingMode{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline void apply(const PackArgs& p, RealType* res, void* context){
    *res=applySeq(p,context);
#ifdef DEBUG_PRINT_OP
    OpWithDynSelectedRoundingMode<OP>::printDebug(p,res);
#endif
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf(*res)) {
      if(isNan(*res)){
	vr_nanHandler();
      }
      if(isinf(*res)){
	vr_infHandler();
      }
    }
#endif
  }

#ifdef DEBUG_PRINT_OP
  static inline void printDebug(const PackArgs& p, const RealType* res){
    static const int nbParam= OP::PackArgs::nb;

    double args[nbParam];
    const double resDouble(*res);
    p.serialyzeDouble(args);
    if(vr_debug_print_op==NULL) return ;
    vr_debug_print_op(nbParam,OP::OpName(), args, &resDouble);
  }
#endif


  static inline RealType applySeq(const PackArgs& p, void* context){
    switch (ROUNDINGMODE) {
    case VR_NEAREST:
      return RoundingNearest<OP>::apply (p);
    case VR_UPWARD:
      return RoundingUpward<OP>::apply (p);
    case VR_DOWNWARD:
      return RoundingDownward<OP>::apply (p);
    case VR_ZERO:
      return RoundingZero<OP>::apply (p);
    case VR_AWAY_ZERO:
      return RoundingAwayZero<OP>::apply (p);
    case VR_RANDOM:
      return RoundingRandom<OP, vr_rand_prng<OP> >::apply (p);
    case VR_RANDOM_DET:
      return RoundingRandom<OP, vr_rand_det<OP> >::apply (p);
    case VR_RANDOM_COMDET:
      return RoundingRandom<OP, vr_rand_comdet<OP> >::apply (p);
    case VR_RANDOM_SCOMDET:
      return RoundingPRandom<OP, vr_rand_scomdet<OP> >::apply (p);
    case VR_SR_MONOTONIC:
      return RoundingSRMonotonic<OP, vr_rand_det<OP> >::apply (p);
    case VR_SR_SMONOTONIC:
      return RoundingSRSMonotonic<OP, vr_rand_det<OP> >::apply (p);
    case VR_AVERAGE:
      return RoundingAverage<OP, vr_rand_prng<OP> >::apply (p);
    case VR_AVERAGE_DET:
      return RoundingAverage<OP,vr_rand_det<OP> >::apply (p);
    case VR_AVERAGE_COMDET:
      return RoundingAverage<OP, vr_rand_comdet<OP> >::apply (p);
    case VR_AVERAGE_SCOMDET:
      return RoundingSAverage<OP, vr_rand_scomdet<OP> >::apply (p);
    case VR_PRANDOM:
      return RoundingPRandom<OP, vr_rand_p<OP,vr_rand_prng> >::apply (p);
    case VR_PRANDOM_DET:
      return RoundingPRandom<OP, vr_rand_p<OP,vr_rand_det > >::apply (p);
    case VR_PRANDOM_COMDET:
      return RoundingPRandom<OP, vr_rand_p<OP,vr_rand_comdet > >::apply (p);
    case VR_FARTHEST:
      return RoundingFarthest<OP>::apply (p);
    case VR_FLOAT:
      return RoundingFloat<OP>::apply (p);
    case VR_NATIVE:
      return RoundingNearest<OP>::apply (p);
    case VR_FTZ:
      vr_panicHandler("FTZ not implemented in backend_verrou");
    }

    return 0;
  }
};

template<class OP>
class OpWithNearestRoundingMode{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline void apply(const PackArgs& p, RealType* res, void* context){
    *res=applySeq(p,context);
#ifdef DEBUG_PRINT_OP
    OpWithDynSelectedRoundingMode<OP>::printDebug(p,res);
#endif
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf(*res)) {
      if(isNan(*res)){
	vr_nanHandler();
      }
      if(isinf(*res)){
	vr_infHandler();
      }
    }
#endif
  }
  static inline RealType applySeq(const PackArgs& p, void* context){
    return RoundingNearest<OP>::apply (p);
  }
};

//#endif
