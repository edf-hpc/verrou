
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdlib.h>

template<class REALTYPE>
class identityFunctor{
public:
  static REALTYPE apply (const REALTYPE& x){return x;};
};

template<class REALTYPE>
class absFunctor{
public:
  static REALTYPE apply(const REALTYPE& x){return std::abs(x);};
};


  
template<class REALTYPE, template <typename T> class FUNCTOR=identityFunctor >
REALTYPE dot(std::vector<REALTYPE> a, std::vector<REALTYPE> b){
  size_t sa=a.size();
  size_t sb=b.size();
#ifdef USE_ASSERT
  assert(sa=sb);
#endif
  
  REALTYPE res=0;
  for(size_t i=0; i< sa;i++){
    REALTYPE va= FUNCTOR<REALTYPE>::apply(a[i]);
    REALTYPE vb= FUNCTOR<REALTYPE>::apply(b[i]);
    res+= va*vb; 
  }
  return res;
}



template<class REALTYPE>
REALTYPE dotConditionNumber(std::vector<REALTYPE>& a, const std::vector<REALTYPE>&b){ 
  REALTYPE res=2.* (dot<REALTYPE,absFunctor>(a,b)/ std::abs(dot<REALTYPE>(b,a)));
#ifdef USE_ASSERT
  assert(res>=2.);
#endif
  return res;
}


template<class REALTYPE>
REALTYPE dotConditionNumberRewrite(std::vector<REALTYPE>& a, const std::vector<REALTYPE>&b){ 
  REALTYPE res=2.* (dot<REALTYPE,absFunctor>(a,b)/ std::abs(dot<REALTYPE>(a,b)));
#ifdef USE_ASSERT
  assert(res>=2.);
#endif
  return res;
}


void feedRandomNumber(std::vector<float> & x, size_t size, size_t seed){
  x.resize(size);
  srand(seed);
  for(size_t i=0; i< size;i++){
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    x[i]=r;
  }
}


int main(int argc, char** argv){

  std::vector<float> a;//={1.,1.1,0.1};
  std::vector<float> b;//={1.,0.1,1.1};
  feedRandomNumber(a,1000,42);
  feedRandomNumber(b,1000,4242);
  
  
#ifndef REWRITE
  std::cout << "cond:" <<  dotConditionNumber(a,b)<<std::endl;
#else
  std::cout << "cond:" <<  dotConditionNumberRewrite(a,b)<<std::endl;
#endif
};

