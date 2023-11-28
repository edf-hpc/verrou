#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <../verrou.h>
#include <iomanip>

#include  <immintrin.h>
#include  <fmaintrin.h>

size_t loopNumber=16;


template<class REALTYPE>
struct loopMaxAbs{
  template<class FUNCTOR1, class FUNCTOR2>
  static REALTYPE sum(FUNCTOR1& f1, FUNCTOR2& f2, size_t size){
    REALTYPE res=0;
    for(size_t i=0; i<size;i++){
      VERROU_SET_SEED(i);
      REALTYPE v1=f1();
      REALTYPE v2=f2();
      res=std::max<REALTYPE>(res, std::abs(v1+v2));
    }
    return res;
  }

  template<class FUNCTOR1, class FUNCTOR2>
  static REALTYPE diff(FUNCTOR1& f1, FUNCTOR2& f2, size_t size){
    REALTYPE res=0;
    for(size_t i=0; i<size;i++){
      VERROU_SET_SEED(i);
      REALTYPE v1=f1();
      REALTYPE v2=f2();
      res=std::max<REALTYPE>(res, std::abs(v1-v2));
    }
    return res;
  }
};


template<class REALTYPE>
struct functorCos{
  REALTYPE _input;
  functorCos(REALTYPE input):_input(input){}

  REALTYPE operator()()const{
    return std::cos(_input);
  };
};

template<class REALTYPE>
struct functorSin{
  REALTYPE _input;
  functorSin(REALTYPE input):_input(input){}

  REALTYPE operator()()const{
    return std::sin(_input);
  };
};

template<class REALTYPE>
struct functorj0{
  REALTYPE _input;
  functorj0(REALTYPE input):_input(input){}

  REALTYPE operator()()const{ return apply(_input);};

  double apply(double x)const{
    return j0(x);
  }
  float apply(float x)const{
    return j0f(x);
  }
};

template<class REALTYPE>
struct functorj1{
  REALTYPE _input;
  functorj1(REALTYPE input):_input(input){}

  REALTYPE operator()()const{ return apply(_input);};

  double apply(double x)const{
    return j1(x);
  }
  float apply(float x)const{
    return j1f(x);
  }
};



template<class REALTYPE>
struct functorPow{
  REALTYPE _input1;
  REALTYPE _input2;
  functorPow(REALTYPE input1, REALTYPE input2):_input1(input1),_input2(input2){}

  REALTYPE operator()()const{
    return std::pow(_input1,_input2);
  };
};

template<class REALTYPE>
struct functorAtan2{
  REALTYPE _input1;
  REALTYPE _input2;
  functorAtan2(REALTYPE input1, REALTYPE input2):_input1(input1),_input2(input2){}

  REALTYPE operator()()const{
    return std::atan2(_input1,_input2);
  };
};


//want to test the equivalence between hardware and libm fma
template<class REALTYPE>
struct functorFmaInstrinsic{
  REALTYPE _input1;
  REALTYPE _input2;
  REALTYPE _input3;
  functorFmaInstrinsic(REALTYPE input1, REALTYPE input2, REALTYPE input3):_input1(input1),_input2(input2),_input3(input3){}

  REALTYPE operator()()const{
    return apply(_input1,_input2,_input3);
  };

  float apply(float a, float b, float c)const{
    float d;
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    bi = _mm_load_ss(&b);
    ci = _mm_load_ss(&c);
    di=_mm_fmadd_ss(ai,bi,ci);
    d=_mm_cvtss_f32(di);
    return d;
  }

  double apply(double a, double b, double c)const{
    double d;
    __m128d ai, bi,ci,di;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmadd_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);
    return d;
  }
};

template<class REALTYPE>
struct functorFma{
  REALTYPE _input1;
  REALTYPE _input2;
  REALTYPE _input3;
  functorFma(REALTYPE input1, REALTYPE input2, REALTYPE input3):_input1(input1),_input2(input2),_input3(input3){}

  REALTYPE operator()()const{
    return apply(_input1,_input2,_input3);
  };
  float apply(float a, float b, float c)const{
    return fmaf(a,b,c);
  }

  double apply(double a, double b, double c)const{
    return fma(a,b,c);
  }
};


template<class REALTYPE>
void checkLibM(){
  {
    functorCos<REALTYPE> cosP(0.1);
    functorCos<REALTYPE> cosN(-0.1);
    REALTYPE resDiff=loopMaxAbs<REALTYPE>::diff(cosP,cosN, loopNumber);
    REALTYPE resErrorCos=resDiff/ cosP();
    std::cout <<"cos: max diff=" << resErrorCos<<std::endl;
  }
  {
    functorSin<REALTYPE> sinP(0.1);
    functorSin<REALTYPE> sinN(-0.1);
    REALTYPE resSum=loopMaxAbs<REALTYPE>::sum(sinP,sinN, loopNumber);
    REALTYPE resErrorSin=resSum/ sinP();
    std::cout <<"sin: max sum=" << resErrorSin<<std::endl;
  }
  {
    functorj0<REALTYPE> j0_P(0.1);
    functorj0<REALTYPE> j0_N(-0.1);
    REALTYPE resDiff=loopMaxAbs<REALTYPE>::diff(j0_P,j0_N, loopNumber);
    REALTYPE resErrorJ0=resDiff/ j0_P();
    std::cout <<"j0: max diff=" << resErrorJ0<<std::endl;
  }
  {
    functorj1<REALTYPE> j1_P(0.1);
    functorj1<REALTYPE> j1_N(-0.1);
    REALTYPE resSum=loopMaxAbs<REALTYPE>::sum(j1_P,j1_N, loopNumber);
    REALTYPE resErrorJ1=resSum/ j1_P();
    std::cout <<"j1: max sum=" << resErrorJ1<<std::endl;
  }
  {
    functorPow<REALTYPE> pow_F(0.1,2.3);
    REALTYPE resDiff=loopMaxAbs<REALTYPE>::diff(pow_F,pow_F, loopNumber);
    REALTYPE resError=resDiff/ pow_F();
    std::cout << "pow: max diff="<< resError<<std::endl;
  }

  {
    functorAtan2<REALTYPE> atan2_v1(0.1,2.3);
    functorAtan2<REALTYPE> atan2_v2(-0.1,2.3);
    functorAtan2<REALTYPE> atan2_nada(-0.1,-2.3);

    REALTYPE resSum=loopMaxAbs<REALTYPE>::sum(atan2_v1,atan2_v2, loopNumber);
    REALTYPE resError=resSum/ atan2_v1();
    std::cout << "atan2: max diff="<< resError<<std::endl;
    REALTYPE resSum2=loopMaxAbs<REALTYPE>::sum(atan2_v1,atan2_nada, loopNumber);
    REALTYPE resError2=resSum2/ atan2_v1();
    std::cout << "atan2: max diff (nada)="<< resError2<<std::endl;
  }

  {
    functorFmaInstrinsic<REALTYPE> fmaI(0.1,0.1,0.1);
    functorFma<REALTYPE> fmaLibm(-0.1,-0.1,0.1);
    REALTYPE resDiff=loopMaxAbs<REALTYPE>::diff(fmaI, fmaLibm, loopNumber);
    std::cout << "fma: max diff ="<< resDiff<<std::endl;
  }
}

int main (int argc, char **argv) {
  std::cout << std::setprecision(17);
  std::cout << "float"<<std::endl;
  checkLibM<float>();
  std::cout << "double"<<std::endl;
  checkLibM<double>();

  return 0;
}
