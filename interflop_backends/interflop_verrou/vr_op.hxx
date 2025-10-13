
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Implementation of error estimation for all FP operations     ---*/
/*---                                                    vr_op.hxx ---*/
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

#include "vr_isNan.hxx"
#include "vr_sqrt.hxx"
#include "cmath"
#include "vr_fma.hxx"
#include <algorithm>

enum opHash : uint32_t{
  addHash=0,
  subHash=1,
  mulHash=2,
  divHash=3,
  maddHash=4,
  castHash=5,
  sqrtHash=6,
  nbOpHash=7
};

enum typeHash : uint32_t{
  floatHash=0,
  doubleHash=1,
  otherHash=2,
  nbTypeHash=3
};


template<class>
inline uint32_t getTypeHash(){return typeHash::otherHash;}
template<>
inline uint32_t getTypeHash<float>(){return typeHash::floatHash;}
template<>
inline uint32_t getTypeHash<double>(){return typeHash::doubleHash;}



template<class REALTYPE, int NB>
struct vr_packArg;


/*
 * takes a real number and returns a uint64_t by reinterpreting its bits, NOT casting it
 * used by the getHash function in the vr_packArg classes
 */
template<class REALTYPE>
inline uint64_t realToUint64_reinterpret_cast(const REALTYPE x)
{
    // insures we have a 64 bits representation
    double x_double = static_cast<double>(x);
    // transmute it to a uint64 by reinterpreting its bits
    // WARNING: this is considered undefined behaviour by the standard
    return *reinterpret_cast<uint64_t*>(&x_double);
}


inline uint32_t realToUint32_reinterpret_cast(const float x)
{
    // insures we have a 64 bits representation
    float x_float = static_cast<float>(x);
    // transmute it to a uint64 by reinterpreting its bits
    // WARNING: this is considered undefined behaviour by the standard
    return *reinterpret_cast<uint32_t*>(&x_float);
}

template<class REALTYPE>
struct vr_packArg<REALTYPE,1>{
  static const int nb= 1;
  typedef REALTYPE RealType;

  inline vr_packArg(const RealType& v1):arg1(v1)
  {
  };


  inline void serialyzeDouble(double* res)const{
    res[0]=(double)arg1;
  }

  inline bool isOneArgNanInf()const{
    return isNanInf<RealType>(arg1);
  }

  const RealType& arg1;
};

template<class REALTYPE>
struct vr_packArg<REALTYPE,2>{
  static const int nb= 2;
  typedef REALTYPE RealType;

  vr_packArg(const RealType& v1,const RealType& v2):arg1(v1),arg2(v2)
  {
  };

  vr_packArg(const std::pair<const REALTYPE&, const REALTYPE&>& pair):arg1(pair.first),arg2(pair.second)
  {
  };

  inline void serialyzeDouble(double* res)const{
    res[0]=(double)arg1;
    res[1]=(double)arg2;
  }

  inline bool isOneArgNanInf()const{
    return (isNanInf<RealType>(arg1) || isNanInf<RealType>(arg2));
  }

  const RealType& arg1;
  const RealType& arg2;
};


template<class REALTYPE>
struct packargsIntReal{
public:

  static const int nb= 2;
  typedef REALTYPE RealType;

  inline packargsIntReal(const int& v1, const REALTYPE& v2):arg1(v1),arg2(v2)
  {
  };


  inline void serialyzeDouble(double* res)const{
    res[0]=(double)arg1;
    res[1]=(double)arg2;
  }

  inline bool isOneArgNanInf()const{
    return isNanInf<RealType>(arg2);
  }

  const int& arg1;
  const RealType& arg2;
};



template<class REALTYPE>
struct vr_packArg<REALTYPE,3>{
  static const int nb= 3;
  typedef REALTYPE RealType;

  vr_packArg(const RealType& v1,const RealType& v2,const RealType& v3):arg1(v1),arg2(v2),arg3(v3){
  };

  inline void serialyzeDouble(double* res)const{
    res[0]=(double)arg1;
    res[1]=(double)arg2;
    res[2]=(double)arg3;
  }

  inline bool isOneArgNanInf()const{
    return (isNanInf<RealType>(arg1) || isNanInf<RealType>(arg2) || isNanInf<RealType>(arg3) );
  }

  const RealType& arg1;
  const RealType& arg2;
  const RealType& arg3;
};

#include <algorithm>


template<class PACKARGS>
struct vr_storeFloat;

template<class REALTYPE>
struct vr_storeFloat<vr_packArg<REALTYPE,1> >{
  vr_storeFloat(const vr_packArg<REALTYPE,1>& p): arg1(p.arg1){
  }
  inline vr_packArg<float,1> getPack()const{
    return vr_packArg<float,1>(arg1);
  }

  const float arg1;
};


template<class REALTYPE>
struct vr_storeFloat<vr_packArg<REALTYPE, 2> >{
  vr_storeFloat(const vr_packArg<REALTYPE,2>& p): arg1(p.arg1 ),
						  arg2(p.arg2 ){
  }
  inline vr_packArg<float,2> getPack()const{
    return vr_packArg<float,2>(arg1,arg2);
  }
  const float arg1;
  const float arg2;
};

template<class REALTYPE>
struct vr_storeFloat<vr_packArg<REALTYPE, 3> >{
  vr_storeFloat(const vr_packArg<REALTYPE,3>& p): arg1(p.arg1 ),
						  arg2(p.arg2 ),
						  arg3(p.arg3 ){
  }
  inline vr_packArg<float,3> getPack()const{
    return vr_packArg<float,3>(arg1,arg2,arg3);
  }
  const float arg1;
  const float arg2;
  const float arg3;
};


template<class REALTYPE>
struct vr_storeFloat<packargsIntReal<REALTYPE> >{
  vr_storeFloat(const packargsIntReal<REALTYPE>& p):arg1(p.arg1),
						    arg2(p.arg2){
  }
  inline packargsIntReal<float> getPack()const{
    return packargsIntReal<float>(arg1,arg2);
  }

  const int arg1;
  const float arg2;
};



template<typename REAL> class SubOp;

template<typename REAL>
class AddOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;
  typedef AddOp<float> FloatOp;
  typedef vr_packArg<float,2> FloatPackArgs;

  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "add";}
  static inline uint32_t getHash(){return (uint32_t)opHash::addHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}

  static inline RealType nearestOp (const PackArgs&  p) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    return a+b;
  }

  static inline RealType error (const PackArgs& p, const RealType& x) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    const RealType z=x-a;
    return ((a-(x-z)) + (b-z)); //algo TwoSum
  }

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return AddOp<RealType>::error(p,c);
  }

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  static inline void check(const PackArgs& p,const RealType & c){
  }

  static inline void twoSum(const RealType& a,const RealType& b, RealType& x, RealType& y){
    const PackArgs p(a,b);
    x=AddOp<REAL>::nearestOp(p);
    y=AddOp<REAL>::error(p,x);
  }


  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
    const PackArgs pnew(pminmax);
    const uint32_t hashOp(AddOp<RealType>::getHash());
    return r.hash(pnew, hashOp);
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    if( p.arg1 >0){//same sign
      if( p.arg2 >0){
	// p.arg1 >0  p.arg2 >0
	const uint32_t hashOp(AddOp<RealType>::getHash());
	const std::pair<const RealType&, const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
	const PackArgs pnew(pminmax);
	return r.hash(pnew, hashOp);
      }else{
	// p.arg1 >0  p.arg2 <0
	const uint32_t hashOp(SubOp<RealType>::getHash());
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,mparg2));
	const PackArgs pnew(pminmax);
	return r.hash(pnew, hashOp);
      }
    }else{//p.arg1<0
      if( p.arg2 >0){
	// p.arg1 <0  p.arg2 >0
	const uint32_t hashOp(SubOp<RealType>::getHash());
	const RealType mparg1(-p.arg1);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,p.arg2));
	const PackArgs pnew(pminmax);
	return r.hashBar(pnew, hashOp);
      }else{
	// p.arg1 <0  p.arg2 <0
	const uint32_t hashOp(AddOp<RealType>::getHash());
	const RealType mparg1(-p.arg1);
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,mparg2));
	const PackArgs pnew(pminmax);
	return r.hashBar(pnew,hashOp);
      }
    }
  }
};


template<typename REAL>
class SubOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;
  typedef SubOp<float> FloatOp;
  typedef vr_packArg<float,2> FloatPackArgs;

  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "sub";}
  static inline uint32_t getHash(){return (uint32_t)opHash::subHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}

  static inline RealType nearestOp (const PackArgs&  p) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    return a-b;
  }

  static inline RealType error (const PackArgs& p, const RealType& x) {
    const RealType & a(p.arg1);
    const RealType & b(-p.arg2);
    const RealType z=x-a;
    return ((a-(x-z)) + (b-z)); //algo TwoSum
  }

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return SubOp<RealType>::error(p,c);
  }


  static inline void check(const PackArgs& p,const RealType & c){
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(SubOp<RealType>::getHash());
    return r.hash(p, hashOp);
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r, const PackArgs& p){
    const RealType p2(-p.arg2);
    return AddOp<RealType>::hashScom(r, PackArgs(p.arg1,p2));
  }

};




//splitFactor used by MulOp
template<class REALTYPE>
constexpr REALTYPE splitFactor();

template<>
constexpr double splitFactor<double>(){
  return 134217729; //((2^27)+1); /27 en double  sup(53/2) /
}

template<>
constexpr float splitFactor<float>(){
  return 4097; //((2^12)+1); / 24/2 en float/
}



template<class REALTYPE>
class ErrorForMul{
public:
  typedef REALTYPE RealType;
  static inline RealType apply(const RealType& a, const RealType& b, const RealType& x){
    /*Provient de "Accurate Sum and dot product" OGITA RUMP OISHI */
#ifdef    USE_VERROU_FMA
    return vr_fma(a,b,-x);
#else
    RealType a1,a2;
    RealType b1,b2;
    ErrorForMul<RealType>::split(a,a1,a2);
    ErrorForMul<RealType>::split(b,b1,b2);

    return (((a1*b1-x)+a1*b2+a2*b1)+a2*b2);
#endif
  }

  static inline void split(RealType a, RealType& x, RealType& y){
    //    const RealType factor=134217729; //((2^27)+1); /*27 en double*/
    constexpr RealType factor(splitFactor<RealType>());
    const RealType c=factor*a;
    x=(c-(c-a));
    y=(a-x);
  }
};


template<class REALTYPE>
class sameSignOfErrorForMul;

template<>
class sameSignOfErrorForMul<double>{
public:
  typedef double RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
#ifdef VERROU_DENORM_HACKS_DOUBLE
    RealType arg1=p.arg1;
    RealType arg2=p.arg2;
    RealType cshift=c;
    constexpr RealType shift(0x1.p500);
    constexpr RealType mone(-1.);
    if(arg1 < 1. && arg1 >mone){
      arg1 *= shift;
      cshift*=shift;
    }
    if(arg2 < 1. && arg2 > mone){
      arg2 *= shift;
      cshift*=shift;
    }
    return ErrorForMul<RealType>::apply(arg1,arg2,cshift);
#else
    return ErrorForMul<RealType>::apply(p.arg1,p.arg2,c);
#endif
  };
};



template<>
class sameSignOfErrorForMul<float>{
public:
  typedef float RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
#ifdef VERROU_DENORM_HACKS_FLOAT
    const double p1d=(double)p.arg1;
    const double p2d=(double)p.arg2;
    const double cd=(double)c;

    const double res(ErrorForMul<double>::apply(p1d,p2d,cd));
    if(res<0){
      return -1.f;
    }
    if(res>0){
      return 1.f;
    }
    return 0.f;
#else
    return ErrorForMul<double>::apply(p.arg1,p.arg2,c);
#endif
  };
};


template<typename REAL>
class MulOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;
  typedef MulOp<float> FloatOp;
  typedef vr_packArg<float,2> FloatPackArgs;

  static const bool sign_denorm_hack_needed=true;

  static const char* OpName(){return "mul";}
  static inline uint32_t getHash(){return (uint32_t)opHash::mulHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}



  static inline RealType nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    return a*b;
  };

  static inline RealType error (const PackArgs& p, const RealType& x) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    return ErrorForMul<RealType>::apply(a,b,x);
  };

  static inline RealType sameSignOfError(const PackArgs& p,const RealType& c) {
    return sameSignOfErrorForMul<RealType>::apply(p,c);
  }

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  static inline void check(const PackArgs& p,const RealType & c){
  };

  static inline void twoProd(const RealType& a,const RealType& b, RealType& x,RealType& y){
    const PackArgs p(a,b);
    x=MulOp<REAL>::nearestOp(p);
    y=MulOp<REAL>::error(p,x);
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
    const PackArgs pnew(pminmax);
    const uint32_t hashOp(MulOp<RealType>::getHash());
    return r.hash(pnew, hashOp);
  }


  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r, const PackArgs& p){
    const uint32_t hashOp(MulOp<RealType>::getHash());
    if( p.arg1 >0 ){
      if( p.arg2 >0 ){
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
	const PackArgs pnew(pminmax);
	return r.hash(pnew,hashOp);
      }else{
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,mparg2));
	const PackArgs pnew(pminmax);
	return r.hashBar(pnew,hashOp);
      }
    }else{//p.arg1 <0
      if( p.arg2 >0){
	const RealType mparg1(-p.arg1);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,p.arg2));
	const PackArgs pnew(pminmax);
	return r.hashBar(pnew,hashOp);
      }else{
	const RealType mparg1(-p.arg1);
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,mparg2));
	const PackArgs pnew(pminmax);
	return r.hash(pnew,hashOp);
      }
    }
  }

};



template<class REALTYPE>
class sameSignOfErrorForDiv{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
    const RealType & x(p.arg1);
    const RealType & y(p.arg2);
#ifdef    USE_VERROU_FMA
    const RealType r=-vr_fma(c,y,-x);
    return r*y;
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(c,y,u,uu);
    return ( x-u-uu)*y ;
#endif
  };
};



template<>
class sameSignOfErrorForDiv<float>{
public:
  typedef float RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
#ifdef VERROU_DENORM_HACKS_FLOAT
    return apply_double(p,c);
#else
    return apply_float(p,c);
#endif
  }

  static inline RealType apply_double (const PackArgs& p,const RealType& c) {
    const double x((double)p.arg1);
    const double y((double) p.arg2);
#ifdef    USE_VERROU_FMA
    const double r=-vr_fma((double)c,y,-x);

    if(r>0){return p.arg2;}
    if(r<0){return -p.arg2;}
    //if(r==0){
    return 0.;
    //}
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(c,y,u,uu);
    return ( x-u-uu)*y ;
#endif
  };

  static inline RealType apply_float (const PackArgs& p,const RealType& c) {
    const RealType x(p.arg1);
    const RealType y(p.arg2);
#ifdef    USE_VERROU_FMA
    const RealType r=-vr_fma(c,y,-x);

    if(r>0){return p.arg2;}
    if(r<0){return -p.arg2;}
    //if(r==0){
    return 0.;
    //}
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(c,y,u,uu);
    return ( x-u-uu)*y ;
#endif
  };
};



template<typename REAL>
class DivOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;
  typedef DivOp<float> FloatOp;
  typedef vr_packArg<float,2> FloatPackArgs;

  static const bool sign_denorm_hack_needed=true;

  static const char* OpName(){return "div";}
  static inline uint32_t getHash(){return (uint32_t)opHash::divHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}


  static RealType inline nearestOp (const PackArgs& p) {
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    return a/b;
  };

  static inline RealType error (const PackArgs& p, const RealType& c) {
    const RealType & x(p.arg1);
    const RealType & y(p.arg2);
#ifdef    USE_VERROU_FMA
    const RealType r=-vr_fma(c,y,-x);
    return r/y;
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(c,y,u,uu);
    return ( x-u-uu)/y ;
#endif
  };

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return sameSignOfErrorForDiv<RealType>::apply(p,c);
  };

  static inline void check(const PackArgs& p,const RealType & c){
  };

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return (isNanInf<RealType>(p.arg1))||(p.arg2==RealType(0.));
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(DivOp<RealType>::getHash());
    return r.hash(p, hashOp);
  }


  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r, const PackArgs& p){
    const uint32_t hashOp(DivOp::getHash());
    if( p.arg1 >0){
      if( p.arg2 >0){
	//p.arg1 >0  p.arg2 >0
	return r.hash(PackArgs(p.arg1, p.arg2), hashOp);
      }else{
	//p.arg1 >0  p.arg2 <0
	const RealType mparg2(- p.arg2);
	return r.hashBar(PackArgs(p.arg1, mparg2), hashOp);
      }
    }else{//p.arg1<0
      if( p.arg2 >0){
	const RealType mparg1(- p.arg1);
	return r.hashBar(PackArgs(mparg1, p.arg2), hashOp);
      }else{
	//p.arg1<0  p.arg2<0
	const RealType mparg1(- p.arg1);
	const RealType mparg2(- p.arg2);
	return r.hash(PackArgs(mparg1, mparg2), hashOp);
      }
    }
  }
};




template<typename REAL>
class MAddOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,3> PackArgs;
  typedef MAddOp<float> FloatOp;
  typedef vr_packArg<float,3> FloatPackArgs;

  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "madd";}
  static inline uint32_t getHash(){return (uint32_t)opHash::maddHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}

  static RealType inline nearestOp (const PackArgs& p) {
#ifdef    USE_VERROU_FMA
    const RealType & a(p.arg1);
    const RealType & b(p.arg2);
    const RealType & c(p.arg3);
    return vr_fma(a,b,c);
#else
    return 0./0.;
#endif
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
    //ErrFmaApp : Exact and Aproximated Error of the FMA By Boldo and Muller
    const RealType & a(p.arg1);
    const RealType & x(p.arg2);
    const RealType & b(p.arg3);

    RealType ph,pl;
    MulOp<RealType>::twoProd(a,x, ph,pl);

    RealType uh,ul;
    AddOp<RealType>::twoSum(b,ph, uh,ul);

    const RealType t(uh-z);
    return (t+(pl+ul)) ;
  };

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return error(p,c) ;
  };

  static inline void check(const PackArgs& p, const RealType& d){
  };

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
    const vr_packArg<RealType,3> pnew(pminmax.first, pminmax.second, p.arg3);
    const uint32_t hashOp(MAddOp::getHash());
    return r.hash(pnew,hashOp);
  }

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    if(p.arg3==0.){
      const vr_packArg<RealType,2> pnew(p.arg1, p.arg2);
      return MulOp<RealType>::hashScom(r,pnew);
    }
    /*  if(p.arg1==1.){
      const vr_packArg<RealType,2> pnew(p.arg2, p.arg3);
      return AddOp<RealType>::hashScom(r,pnew);
    }
    if(p.arg2==1.){
      const vr_packArg<RealType,2> pnew(p.arg1, p.arg3);
      return AddOp<RealType>::hashScom(r,pnew);
    }
    if(p.arg1==-1.){
      const RealType p2(-p.arg2);
      const vr_packArg<RealType,2> pnew(p2, p.arg3);
      return AddOp<RealType>::hashScom(r,pnew);
    }
    if(p.arg2==-1.){
      const RealType p1(-p.arg1);
      const vr_packArg<RealType,2> pnew(p1, p.arg3);
      return AddOp<RealType>::hashScom(r,pnew);
      }*/

    const uint32_t hashOp(MAddOp::getHash());
    if(p.arg1 >0){
      if(p.arg2 >0){
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,p.arg2));
	if(p.arg3 >0){
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,p.arg3);
	  //evenNumPositive: False
	  return r.hash( pnew,hashOp);
	}else{//p.arg3<0
	  //evenNumPositive: True
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,p.arg3);
	  return r.hashBar(pnew,hashOp);
	}
      }else{//p.arg2<0
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(p.arg1,mparg2));
	if(p.arg3 >0){
	  //evenNumPositive: True
	  const RealType mparg3(-p.arg3);
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,mparg3);
	  return r.hash(pnew,hashOp);
	}else{//p.arg3<0
	  //evenNumPositive: False
	  const RealType mparg3(-p.arg3);
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,mparg3);
	  return r.hashBar(pnew, hashOp);
	}
      }
    }else{//p.arg1<0
      const RealType mparg1(-p.arg1);
      if(p.arg2 >0){
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,p.arg2));
	if(p.arg3 >0){
	  //evenNumPositive: True
	  const RealType mparg3(-p.arg3);
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,mparg3);
	  return r.hash(pnew,hashOp);
	}else{//p.arg3<0
	  //evenNumPositive: False
	  const RealType mparg3(-p.arg3);
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,mparg3);
	  return r.hashBar(pnew, hashOp);
	}
      }else{//p.arg1<0 p.arg2<0
	const RealType mparg2(-p.arg2);
	const std::pair<const RealType&,const RealType&> pminmax(std::minmax(mparg1,mparg2));
	if(p.arg3 >0){
	  //evenNumPositive: False
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,p.arg3);
	  return r.hash( pnew,hashOp);
	}else{//p.arg3<0
	  //evenNumPositive: True
	  const vr_packArg<RealType,3> pnew(pminmax.first,pminmax.second,p.arg3);
	  return r.hashBar(pnew,hashOp);
	}
      }
    }
  }
};



template<typename REALINPUT, typename REALOUTPUT>
class CastOp{
public:
  typedef REALINPUT RealTypeIn;
  typedef REALOUTPUT RealTypeOut;
  typedef RealTypeOut RealType;
  typedef vr_packArg<RealTypeIn,1> PackArgs;
  typedef CastOp<float,float> FloatOp;
  static const bool sign_denorm_hack_needed=false;

  static const char* OpName(){return "cast";}
  static inline uint32_t getHash(){return (uint32_t)opHash::castHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}

  static inline RealTypeOut nearestOp (const PackArgs& p) {
    const RealTypeIn & in(p.arg1);
    return (RealTypeOut)in;
  };

  static inline RealTypeOut error (const PackArgs& p, const RealTypeOut& z) {
    const RealTypeIn & a(p.arg1);
    const RealTypeIn errorHo= a- (RealTypeIn)z;
    return (RealTypeOut) errorHo;
  };

  static inline RealTypeOut sameSignOfError (const PackArgs& p,const RealTypeOut& c) {
    return error(p,c) ;
  };

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  static inline void check(const PackArgs& p, const RealTypeOut& d){
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(CastOp::getHash());
    return r.hash(p, hashOp);
  }


  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(CastOp::getHash());
    if( p.arg1 >0){
      return r.hash(p, hashOp);
    }else{
      const RealType mparg1(-p.arg1);
      return r.hashBar(PackArgs(mparg1), hashOp);
    }
  }

};


#ifdef    USE_VERROU_SQRT

template<class REALTYPE>
class sameSignOfErrorForSqrt{
public:
  typedef REALTYPE RealType;
  typedef vr_packArg<RealType,1> PackArgs;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
    const RealType a(p.arg1) ;
    const RealType x(c);
#ifdef    USE_VERROU_FMA
    return vr_fma(-x,x,a) ;
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(x,x,u,uu);
    return ((a-u)-uu);
#endif
  };
};



template<>
class sameSignOfErrorForSqrt<float>{
public:
  typedef float RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  static const bool sign_denorm_hack_needed=true;

  static inline RealType apply (const PackArgs& p,const RealType& c) {
#ifndef VERROU_DENORM_HACKS_FLOAT
    return apply_float(p,c);
#else
    return apply_double(p,c);
#endif
  }

  static inline RealType apply_double (const PackArgs& p,const RealType& c) {
    const double a(p.arg1) ;
    const double x(c);
#ifdef    USE_VERROU_FMA
    const double res=vr_fma(-x,x,a) ;
#else
    double u,uu;
    MulOp<double>::twoProd(x,x,u,uu);
    const double res ((a-u)-uu) ;
#endif
    if(res==0.) return 0.;
    if(res<0) return -1.;
    return 1.; //to avoid flush to zero during double to float cast
  };

  static inline RealType apply_float (const PackArgs& p,const RealType& c) {
    const float a(p.arg1) ;
    const float x(c);
#ifdef    USE_VERROU_FMA
    return vr_fma(-x,x, a) ;
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(x,x,u,uu);
    return ((a-u)-uu) ;
#endif
  };
};




template<typename REAL>
class SqrtOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,1> PackArgs;
  typedef SqrtOp<float> FloatOp;
  typedef vr_packArg<float,1> FloatPackArgs;

  static const bool sign_denorm_hack_needed=true;

  static const char* OpName(){return "sqrt";}
  static inline uint32_t getHash(){return (uint32_t)opHash::sqrtHash * (uint32_t)typeHash::nbTypeHash + (uint32_t)getTypeHash<RealType>();}

  static inline RealType nearestOp (const PackArgs& p) {
    return vr_sqrt<RealType>(p.arg1);
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
    const RealType & a(p.arg1);
#ifdef    USE_VERROU_FMA
    return vr_fma(-z,z,a) / (2.*z);
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(z,z,u,uu);
    return ((a-u)-uu)  / (2.*z);
#endif
  };

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& z) {
    return sameSignOfErrorForSqrt<RealType>::apply(p,z);
  };

  static inline bool isInfNotSpecificToNearest(const PackArgs&p){
    return p.isOneArgNanInf();
  }

  static inline void check(const PackArgs& p, const RealType& d){
  };

  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashCom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(SqrtOp::getHash());
    return r.hash(p, hashOp);
  }


  template<class RANDSCOM>
  static inline typename RANDSCOM::TypeOut hashScom(const RANDSCOM& r,const PackArgs& p){
    const uint32_t hashOp(SqrtOp::getHash());
    return r.hash(p, hashOp);
  }

};

#endif //USE_VERROU_SQRT
