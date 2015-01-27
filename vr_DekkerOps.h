#pragma once
#include "vr_fpRepr.hxx" 


template<class REALTYPE> 
REALTYPE splitFactor(){
  return 0./ 0.; //nan to be sur not used
} 

template<>
double splitFactor<double>(){
  return 134217729; //((2^27)+1); /*27 en double  sup(53/2) */ 
}

template<>
float splitFactor<float>(){
  return 4097; //((2^12)+1); /*24/2 en float*/ 
}



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
  
  static void twoSum(RealType a, RealType b, RealType& x, RealType& y){
    x=a+b;
    RealType z=x-a;
    y=(a-(x-z)) + (b-z);
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

    const RealType constant=splitFactor<RealType>();
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

  }
  
  /*Provient de "Accurate Sum and dot product" OGITA RUMP OISHI */
  static void split(RealType a, RealType& x, RealType& y){
    //    const RealType factor=134217729; //((2^27)+1); /*27 en double*/ 
    const RealType factor=splitFactor<RealType>();
    RealType c=factor*a;
    x=(c-(c-a));
    y=(a-x);
  }
  
  static void twoProd(RealType a, RealType b, RealType& x, RealType& y){
    RealType a1,a2;
    RealType b1,b2;
    x=a*b;
    split(a,a1,a2);
    split(b,b1,b2);
    
    //    y=( a2*b2-( ( (x-a1*b1) -a2*b1)- a1*b2 ));
    y=((a1*b1-x)+a1*b2+a2*b1)+a2*b2;
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
    twoProd(c,y,u,uu);
    cc=( x-u-uu)/y ;
    z=c;
    zz=cc;
    /*    z=c+cc;
	  zz=c-z+cc;*/
  }


};
