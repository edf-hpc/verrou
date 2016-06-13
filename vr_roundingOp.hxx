
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Implementation of the software implementation of rounding    ---*/
/*--- mode switching.                                              ---*/
/*---                                            vr_roundingOp.hxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2016
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#pragma once

extern vrRand vr_rand;
extern vr_RoundingMode ROUNDINGMODE;

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
class RoundingRandom{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p ){
    RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    const RealType signError=OP::sameSignOfError(p,res);
    if(signError==0.){
      return res;
    }else{
      const bool doNoChange=(vr_rand.getBool() );
      if(doNoChange){
	return res;
      }else{
	if(signError>0){
	  return nextAfter<RealType>(res);
	}else{
	  return nextPrev<RealType>(res);
	}
      }
    }
  } ;
};



template<class OP>
class RoundingAverage{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    const RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    const RealType error=OP::error(p,res);
    if(error==0.){
      return res;
    }
    const int s = error>=0 ? 1 : -1;
    const RealType u =ulp(res);
    bool doNotChange= ((vr_rand.getRandomInt() * u) > (vr_rand.getRandomIntMax() * s * error));

    if(doNotChange){
      return res;
    }else{
      if(error>0){
	return nextAfter<RealType>(res);
      }
      if(error<0){
	return nextPrev<RealType>(res);
      }
      return res;
    }
  } ;
};



template<class OP>
class RoundingZero{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;

  static inline RealType apply(const PackArgs& p){
    RealType res=OP::nearestOp(p) ;
    OP::check(p,res);
    const RealType signError=OP::sameSignOfError(p,res);

    if(signError>0 && res <0){
      return nextAfter<RealType>(res);
    }
    if(signError<0 && res >0){
      return nextPrev<RealType>(res);
    }
    return res;
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
    const RealType signError=OP::sameSignOfError(p,res);

    if(signError>0.){
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
    const RealType signError=OP::sameSignOfError(p,res);

    if(signError<0){
      return nextPrev<RealType>(res);
    }
    return res;
  } ;
};




template<class OP>
class OpWithSelectedRoundingMode{
public:
  typedef typename OP::RealType RealType;
  typedef typename OP::PackArgs PackArgs;


  inline
  static RealType apply(const PackArgs& p){
    switch (ROUNDINGMODE) {
    case VR_NEAREST:
      return RoundingNearest<OP>::apply (p);
    case VR_UPWARD:
      return RoundingUpward<OP>::apply (p);
    case VR_DOWNWARD:
      return RoundingDownward<OP>::apply (p);
    case VR_ZERO:
      return RoundingZero<OP>::apply (p);
    case VR_RANDOM:
      return RoundingRandom<OP>::apply (p);
    case VR_AVERAGE:
      return RoundingAverage<OP>::apply (p);
    }
    return 0;
  }


};
