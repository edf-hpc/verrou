
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Implementation of error estimation for all FP operations     ---*/
/*---                                                    vr_op.hxx ---*/
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


template<class REALTYPE, int NB>
struct vr_packArg{
  static const int nb= NB;
  typedef REALTYPE RealType;

  vr_packArg(const RealType& v1){
    args[0]=v1;
  };

  vr_packArg(const RealType& v1, const RealType& v2){
    args[0]=v1; args[1]=v2;
  };

  vr_packArg(const RealType& v1, RealType& v2, RealType& v3){
    args[0]=v1; args[1]=v2; args[2]=v3;
  };

  RealType args[NB];

};




template<typename REAL>
class AddOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static RealType inline nearestOp (const PackArgs&  p) {
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    return a+b;
  }

  static inline RealType error (const PackArgs& p, const RealType& x) {
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    const RealType z=x-a;
    return ((a-(x-z)) + (b-z)); //algo TwoSum
  }

  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return AddOp<RealType>::error(p,c);
  }


  static inline void check(const PackArgs& p,const RealType & c){
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);

    vr_checkCancellation (a, b, c);
  }

  static inline void twoSum(const RealType& a,const RealType& b, RealType& x,RealType& y ){
    const PackArgs p(a,b);
    x=AddOp<REAL>::nearestOp(p);
    y=AddOp<REAL>::error(p,x);
  }


};



//splitFactor used by MulOp
template<class REALTYPE>
REALTYPE splitFactor(){
  return 0./ 0.; //nan to make sure not used
}

template<>
double splitFactor<double>(){
  return 134217729; //((2^27)+1); /27 en double  sup(53/2) /
}

template<>
float splitFactor<float>(){
  return 4097; //((2^12)+1); / 24/2 en float/
}




template<typename REAL>
class MulOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static RealType inline nearestOp (const PackArgs& p) {
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    return a*b;
  };

  static inline RealType error (const PackArgs& p, const RealType& x) {
    /*Provient de "Accurate Sum and dot product" OGITA RUMP OISHI */
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    //    return __builtin_fma(a,b,-x);
    //    VG_(umsg)("vr_fma \n");
#ifdef    USE_VERROU_FMA
    RealType c;
    c=vr_fma(a,b,-x);
    return c;
#else
    RealType a1,a2;
    RealType b1,b2;
    MulOp<RealType>::split(a,a1,a2);
    MulOp<RealType>::split(b,b1,b2);

    return (((a1*b1-x)+a1*b2+a2*b1)+a2*b2);
#endif
  };




  static inline void split(RealType a, RealType& x, RealType& y){
    //    const RealType factor=134217729; //((2^27)+1); /*27 en double*/
    const RealType factor(splitFactor<RealType>());
    RealType c=factor*a;
    x=(c-(c-a));
    y=(a-x);
  }


  static inline RealType sameSignOfError (const PackArgs& p,const RealType& c) {
    return MulOp<RealType>::error(p,c);
  };


  static inline void check(const PackArgs& p,const RealType & c){
  };

  static inline void twoProd(const RealType& a,const RealType& b, RealType& x,RealType& y ){
    const PackArgs p(a,b);
    x=MulOp<REAL>::nearestOp(p);
    y=MulOp<REAL>::error(p,x);
  }

};


template<typename REAL>
class DivOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,2> PackArgs;

  static RealType inline nearestOp (const PackArgs& p) {
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    return a/b;
  };

  static inline RealType error (const PackArgs& p, const RealType& c) {
    const RealType & x(p.args[0]);
    const RealType & y(p.args[1]);
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
    const RealType & x(p.args[0]);
    const RealType & y(p.args[1]);
#ifdef    USE_VERROU_FMA
    const RealType r=-vr_fma(c,y,-x);
    return r*y;
#else
    RealType u,uu;
    MulOp<RealType>::twoProd(c,y,u,uu);
    return ( x-u-uu)*y ;
#endif
  };


  static inline void check(const PackArgs& p,const RealType & c){
  };

};


template<typename REAL>
class MAddOp{
public:
  typedef REAL RealType;
  typedef vr_packArg<RealType,3> PackArgs;

  static RealType inline nearestOp (const PackArgs& p) {
#ifdef    USE_VERROU_FMA
    const RealType & a(p.args[0]);
    const RealType & b(p.args[1]);
    const RealType & c(p.args[2]);
    return vr_fma(a,b,c);
#else
    return 0./0.;
#endif
  };

  static inline RealType error (const PackArgs& p, const RealType& z) {
    //ErrFmaApp : Exact and Aproximated Error of the FMA By Boldo and Muller
    const RealType & a(p.args[0]);
    const RealType & x(p.args[1]);
    const RealType & b(p.args[2]);

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

};



template<typename REALINPUT, typename REALOUTPUT>
class CastOp{
public:
  typedef REALINPUT RealTypeIn;
  typedef REALOUTPUT RealTypeOut;
  typedef RealTypeOut RealType;
  typedef vr_packArg<RealTypeIn,1> PackArgs;

  static RealTypeOut inline nearestOp (const PackArgs& p) {
    const RealTypeIn & in(p.args[0]);
    return (RealTypeOut)in;
  };

  static inline RealTypeOut error (const PackArgs& p, const RealTypeOut& z) {
    const RealTypeIn & a(p.args[0]);
    const RealTypeIn errorHo= a- (RealTypeIn)z;
    return (RealTypeOut) errorHo;
  };

  static inline RealTypeOut sameSignOfError (const PackArgs& p,const RealTypeOut& c) {
    return error(p,c) ;
  };


  static inline void check(const PackArgs& p, const RealTypeOut& d){
  };

};
