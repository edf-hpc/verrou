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

