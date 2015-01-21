#pragma once
#include "vr_fpRepr.hxx" 



template<class REALTYPE>
	   class DekkerOp {
  /* Implementation of the paper of Dekker 1971 :A floating point technique for extending the Available precision*/

public:
  typedef REALTYPE RealType;

  

  static RealType abs(RealType a){    
    if(a>0){
      return a;
    }else{
      return -a;
    }
  };

  static void sum12(RealType x, RealType y, RealType& z, RealType& zz){
    z=x+y;
    if(abs(x) > abs(y)){
      zz= y -(z-x);
    }else{
      zz=x-(z-y);
    }
    
  };
  
  
  static void priest(RealType a, RealType b, RealType& c, RealType& d){
    //version alternative de la somme : pour test
    // Priest's algorithm
    //
    // Priest, D. M.: 1992, "On Properties of Floating Point Arithmetics: Numerical
    // Stability and the Cost of Accurate Computations". Ph.D. thesis, Mathematics
    // Department, University of California, Berkeley, CA, USA.
    //
    // ftp://ftp.icsi.berkeley.edu/pub/theory/priest-thesis.ps.Z   

    c = a + b;
    const RealType e = c - a;
    const RealType g = c - e;
    const RealType h = g - a;
    const RealType f = b - h;
    d = f - e;

    if (d + e != f) {
      c = a;
      d = b;
    }
  };


  static void mul12(RealType x, RealType y, RealType& z, RealType& zz){
    RealType p;
    //static
    int t=(1+storedBits(p));
    //int t=-2;//(storedBits(p));
      //static
    RealType constant=(2^(t-t/2))+1;
    p= x*constant;
    RealType hx=x-p+p;
    RealType tx=x-hx;

    p=y*constant;
    RealType hy=y-p+p;
    RealType ty=y-hy;
    
    p=hx*hy;
    RealType q=hx*ty+tx*hy;

    z=p+q;
    zz=p-z+q+tx*ty;    

    //    z*=2; //to ckeck explosion
  }
  

  
  /*  static void mul2(RealType x, RealType xx, RealType y,RealType yy, RealType& z, RealType& zz){
    RealType c,cc;
    mul12(x,y,c,cc);
    cc= x*yy+xx*y+cc;
    z=c+cc;
    zz=c-z+cc;
    }*/

    
  static void div12(RealType x, RealType y, RealType& z, RealType& zz){
    RealType c,cc,u,uu;
    c=x/y;
    mul12(c,y,u,uu);
    cc=( x-u-uu)/y ;
    z=c+cc;
    zz=c-z+cc;
  }


};
