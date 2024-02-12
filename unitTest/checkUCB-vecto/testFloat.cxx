#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>


#ifdef __x86_64__
#include  <immintrin.h>
#endif

#ifdef __arch64__
#include <arm_neon.h>
#include "sse2neon.h"
#endif

#include <fenv.h>


//Basic Floating point Operation in simple precision
struct Add32{
  typedef float RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("adds");}
  static RealType apply(RealType a, RealType b){return a+b;}
  static __m128 applySSE(__m128 ai, __m128 bi){return _mm_add_ps(ai,bi);};
#ifdef HAVE_AVX
  static __m256 applyAVX(__m256 ai, __m256 bi){return _mm256_add_ps(ai,bi);};
#endif
};

struct Mul32{
  typedef float RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("muls");}
  static RealType apply(RealType a, RealType b){return a*b;}
  static __m128 applySSE(__m128 ai, __m128 bi){return _mm_mul_ps(ai,bi);};
#ifdef HAVE_AVX
  static __m256 applyAVX(__m256 ai, __m256 bi){return _mm256_mul_ps(ai,bi);};
#endif
};

struct Sub32{
  typedef float RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("subs");}
  static RealType apply(RealType a, RealType b){return a-b;}
  static __m128 applySSE(__m128 ai, __m128 bi){return _mm_sub_ps(ai,bi);};
#ifdef HAVE_AVX
  static __m256 applyAVX(__m256 ai, __m256 bi){return _mm256_sub_ps(ai,bi);};
#endif
};

struct Div32{
  typedef float RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("divs");}
  static RealType apply(RealType a, RealType b){return a/b;}
  static __m128 applySSE(__m128 ai, __m128 bi){return _mm_div_ps(ai,bi);};
#ifdef HAVE_AVX
  static __m256 applyAVX(__m256 ai, __m256 bi){return _mm256_div_ps(ai,bi);};
#endif
};

struct Sqrt32{
  typedef float RealType;
  static const int nbArgs=1;
  static std::string name(){return std::string("sqrts");}
  static RealType apply(RealType a){
    float atab[4];
    atab[0]=a;
    __m128 ai= _mm_loadu_ps(atab);
    __m128 ri= _mm_sqrt_ss(ai);
    float res[4];
    _mm_storeu_ps(res,ri);;
    return res[0];
  }
  static __m128 applySSE(__m128 ai){return _mm_sqrt_ps(ai);};
#ifdef HAVE_AVX
  static __m256 applyAVX(__m256 ai){return _mm256_sqrt_ps(ai);};
#endif
};


//Basic Floating point Operation in double precision
struct Add64{
  typedef double RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("addd");}
  static RealType apply(RealType a, RealType b){return a+b;}
  static __m128d applySSE(__m128d ai, __m128d bi){return _mm_add_pd(ai,bi);};
#ifdef HAVE_AVX
  static __m256d applyAVX(__m256d ai, __m256d bi){return _mm256_add_pd(ai,bi);};
#endif
};

struct Mul64{
  typedef double RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("muld");}
  static RealType apply(RealType a, RealType b){return a*b;}
  static __m128d applySSE(__m128d ai, __m128d bi){return _mm_mul_pd(ai,bi);};
#ifdef HAVE_AVX
  static __m256d applyAVX(__m256d ai, __m256d bi){return _mm256_mul_pd(ai,bi);};
#endif
};

struct Sub64{
  typedef double RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("subd");}
  static RealType apply(RealType a, RealType b){return a-b;}
  static __m128d applySSE(__m128d ai, __m128d bi){return _mm_sub_pd(ai,bi);};
#ifdef HAVE_AVX
  static __m256d applyAVX(__m256d ai, __m256d bi){return _mm256_sub_pd(ai,bi);};
#endif
};

struct Div64{
  typedef double RealType;
  static const int nbArgs=2;
  static std::string name(){return std::string("divd");}
  static RealType apply(RealType a, RealType b){return a/b;}
  static __m128d applySSE(__m128d ai, __m128d bi){return _mm_div_pd(ai,bi);};
#ifdef HAVE_AVX
  static __m256d applyAVX(__m256d ai, __m256d bi){return _mm256_div_pd(ai,bi);};
#endif
};

struct Sqrt64{
  typedef double RealType;
  static const int nbArgs=1;
  static std::string name(){return std::string("sqrtd");}
  static RealType apply(RealType a){
    double atab[2];
    atab[0]=a;
    __m128d ai= _mm_loadu_pd(atab);
    __m128d ri= _mm_sqrt_sd(ai,ai);
    double res[2];
    _mm_storeu_pd(res,ri);;
    return res[0];
  }
  static __m128d applySSE(__m128d ai){return _mm_sqrt_pd(ai);};
#ifdef HAVE_AVX
  static __m256d applyAVX(__m256d ai){return _mm256_sqrt_pd(ai);};
#endif
};



//Loop over std::vector
template<class OP,class REALTYPE, int NBARGS>
struct Loop{
};
//Loop over std::vector :: Simple precision
template<class OP>
struct Loop<OP,float,2>{
  typedef std::vector<float> vectType;

  static void applyScalar(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i++){
      res[i]=OP::apply(v1[i],v2[i]);
    }
  }

  static void applySSE(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=4){
      __m128 ai, bi,ri;
      ai = _mm_loadu_ps(&(v1[i])); // unaligned version : performance is not the test purpose
      bi = _mm_loadu_ps(&(v2[i]));
      ri= OP::applySSE(ai,bi);
      _mm_storeu_ps(&res[i],ri);
    }
  }
#ifdef HAVE_AVX
  static void applyAVX(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=8){
      __m256 ai, bi,ri;
      ai = _mm256_loadu_ps(&(v1[i]));
      bi = _mm256_loadu_ps(&(v2[i]));
      ri= OP::applyAVX(ai,bi);
      _mm256_storeu_ps(&res[i],ri);
    }
  }
#endif
};


template<class OP>
struct Loop<OP,float,1>{
  typedef std::vector<float> vectType;
  static void applyScalar(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i++){
      res[i]=OP::apply(v1[i]);
    }
  }
  static void applyScalar(vectType& res, const vectType& v1, const vectType& v2){
    applyScalar(res, v1);
  }

  static void applySSE(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=4){
      __m128 ai,ri;
      ai = _mm_loadu_ps(&(v1[i])); // unaligned version : performance is not the test purpose 
      ri= OP::applySSE(ai);
      _mm_storeu_ps(&res[i],ri);
    }
  }
  static void applySSE(vectType& res, const vectType& v1, const vectType& v2){
    applySSE(res, v1);
  }
#ifdef HAVE_AVX
  static void applyAVX(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=8){
      __m256 ai, ri;
      ai = _mm256_loadu_ps(&(v1[i]));
      ri= OP::applyAVX(ai);
      _mm256_storeu_ps(&res[i],ri);
    }
  }

  static void applyAVX(vectType& res, const vectType& v1, const vectType& v2){
    applyAVX(res, v1);
  }
#endif
};

//Loop over std::vector :: Double precision
template<class OP>
struct Loop<OP,double,2>{
  typedef std::vector<double> vectType;

  static void applyScalar(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i++){
      res[i]=OP::apply(v1[i],v2[i]);
    }
  }

  static void applySSE(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=2){
      __m128d ai, bi,ri;
      ai = _mm_loadu_pd(&(v1[i]));
      bi = _mm_loadu_pd(&(v2[i]));
      ri= OP::applySSE(ai,bi);
      _mm_storeu_pd(&res[i],ri);
    }
  }

#ifdef HAVE_AVX
  static void applyAVX(vectType& res, const vectType& v1, const vectType&v2){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=4){
      __m256d ai, bi,ri;
      ai = _mm256_loadu_pd(&(v1[i]));
      bi = _mm256_loadu_pd(&(v2[i]));
      ri= OP::applyAVX(ai,bi);
      _mm256_storeu_pd(&res[i],ri);
    }
  }
#endif
};

template<class OP>
struct Loop<OP,double,1>{
  typedef std::vector<double> vectType;
  static void applyScalar(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i++){
      res[i]=OP::apply(v1[i]);
    }
  }
  static void applyScalar(vectType& res, const vectType& v1, const vectType& v2){
    applyScalar(res,v1);
  }

  static void applySSE(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=2){
      __m128d ai, ri;
      ai = _mm_loadu_pd(&(v1[i]));
      ri= OP::applySSE(ai);
      _mm_storeu_pd(&res[i],ri);
    }
  }
  static void applySSE(vectType& res, const vectType& v1, const vectType& v2){
    applySSE(res,v1);
  }

#ifdef HAVE_AVX
  static void applyAVX(vectType& res, const vectType& v1){
    int size=(int)v1.size();
    for(int i=0; i< size;i+=4){
      __m256d ai,ri;
      ai = _mm256_loadu_pd(&(v1[i]));
      ri= OP::applyAVX(ai);
      _mm256_storeu_pd(&res[i],ri);
    }
  }
  static void applyAVX(vectType& res, const vectType& v1, const vectType& v2){
    applyAVX(res,v1);
  }
#endif
};

//Basic conversion

// realType to Hex
std::string realTypeToHex(float a){
  std::ostringstream res ;
  res << std::hex << std::uppercase << *(reinterpret_cast<uint32_t*>(&a));
  return res.str() ;
}
std::string realTypeToHex(double a){
  std::ostringstream res ;
  res << std::hex << std::uppercase << *(reinterpret_cast<uint64_t*>(&a)) ;
  return res.str() ;
}


// Hex to RealType
float convertStringToRealType(std::string str){
  char buff[11];
  buff[0]='0';
  buff[1]='x';
  for(int i=0;i<8;i++)
    buff[2+i]=str[i];
  buff[10]='\0';
  uint32_t res;
  sscanf(buff, "%x",&res);
  return *(reinterpret_cast<float*>(&res));
}

double convertStringToRealType(std::string str, std::string str2){
  if(str==std::string("0")){
    str=std::string("00000000");
  }
  if(str2==std::string("0")){
    str2=std::string("00000000");
  }
  if(str2==std::string("1")){
    str2=std::string("00000001");
  }

  char buff[19];
  buff[0]='0';
  buff[1]='x';
  buff[10]='\0';
  for(int i=0;i<8;i++)
    buff[2+i]=str[i];
  uint32_t res1;
  sscanf(buff, "%x",&res1);

  uint32_t res2;
  for(int i=0;i<8;i++)
    buff[2+i]=str2[i];
  sscanf(buff, "%x",&res2);

  uint64_t res= (((uint64_t)res1) << 32)+(uint64_t)res2;

  double resDouble=*(reinterpret_cast<double*>(&res));
  return resDouble;
}




//Check if two floating point are equals
bool isRealTypeEqual(const float& ref,
		     const float& valueToTest){

  //binary check
#ifdef BINARY_CHECK
  return *reinterpret_cast<const int32_t*>(&ref)== *reinterpret_cast<const int32_t*>(&valueToTest);
#else
  //float check
  if(ref!=ref){//Nan expected
    if(valueToTest!=valueToTest){
      return true;
    }
    return false;
  }else{
    return (valueToTest==ref);
  }
#endif
}






//Check if  testComputationTabSeq and refTab are equal : other variables are usefull to print debug info in case of failure
template<class RealType>
bool checkTab(const std::string& fileName,
	      const std::string& computationType, // Seq, SSE,AVX
	      const std::vector<RealType>& testComputationTabSeq,
	      const std::vector<RealType>& refTab,
	      const std::vector<int>& lineTab,
	      const std::vector<RealType>& v1Tab,
	      const std::vector<RealType>& v2Tab){
  int ok=0,ko=0;
  int size=(int)refTab.size();
  for(int i=0;i<size;i++){
    bool isEqual=isRealTypeEqual(refTab[i],testComputationTabSeq[i]);
    if(isEqual){
      ok++;
    }else{
      ko++;
      std::cout << fileName << ":"<<lineTab[i]<< " "<<computationType<< " "
		<<" v1  v2  resRef res \t"
		<<v1Tab[i] << "("<< realTypeToHex(v1Tab[i])<<")\t"
		<<v2Tab[i] << "("<< realTypeToHex(v2Tab[i])<<")\t"
		<<refTab[i] << "("<< realTypeToHex(refTab[i])<<")\t"
		<< testComputationTabSeq[i]<< "("<<realTypeToHex(testComputationTabSeq[i])<<")"<< std::endl;
      //      return;
    }
  }
  std::cout <<fileName<<  " ok:" << ok <<"\tko:" <<ko <<std::endl<<std::endl;
  return (ko==0 && ok!=0);
}

//Check if  testComputationTabSeq and refTab are equal : other variables are usefull to print debug info in case of failure
template<class RealType>
bool checkTab(const std::string& fileName,
	      const std::string& computationType, // Seq, SSE,AVX
	      const std::vector<RealType>& testComputationTabSeq,
	      const std::vector<RealType>& refTab,
	      const std::vector<int>& lineTab,
	      const std::vector<RealType>& v1Tab){
  int ok=0,ko=0;
  int size=(int)refTab.size();
  for(int i=0;i<size;i++){
    bool isEqual=isRealTypeEqual(refTab[i],testComputationTabSeq[i]);
    if(isEqual){
      ok++;
    }else{
      ko++;
      std::cout << fileName << ":"<<lineTab[i]<< " "<<computationType<< " "
		<<" v1  resRef res \t"
		<<v1Tab[i] << "("<< realTypeToHex(v1Tab[i])<<")\t"
		<<refTab[i] << "("<< realTypeToHex(refTab[i])<<")\t"
		<< testComputationTabSeq[i]<< "("<<realTypeToHex(testComputationTabSeq[i])<<")"<< std::endl;
      //      return;
    }
  }
  std::cout <<fileName<<  " ok:" << ok <<"\tko:" <<ko <<std::endl<<std::endl;
  return (ko==0 && ok!=0);
}


//Parse the end of the line of the UCB format : float
void parseEndUCBLine(std::istringstream& iss, float& v1, float& v2, float& ref ){
  std::string v1Str,v2Str,refStr;
  iss >> v1Str >> v2Str>> refStr;

  v1=convertStringToRealType(v1Str);
  v2=convertStringToRealType(v2Str);
  ref=convertStringToRealType(refStr);
}

//Parse the end of the line of the UCB format : double
void parseEndUCBLine(std::istringstream& iss, double& v1, double& v2, double& ref ){
  std::string v1StrL,v2StrL,refStrL;
  std::string v1StrH,v2StrH,refStrH;
  iss >> v1StrL >> v1StrH
      >> v2StrL >> v2StrH
      >> refStrL >> refStrH;

  v1=convertStringToRealType(v1StrL, v1StrH);
  v2=convertStringToRealType(v2StrL, v2StrH );
  ref=convertStringToRealType(refStrL, refStrH);
}


//Parse the end of the line of the UCB format : float
void parseEndUCBLine_unary(std::istringstream& iss, float& v1, float& ref ){
  std::string v1Str,refStr;
  iss >> v1Str >> refStr;

  v1=convertStringToRealType(v1Str);
  ref=convertStringToRealType(refStr);
}

//Parse the end of the line of the UCB format : double
void parseEndUCBLine_unary(std::istringstream& iss, double& v1, double& ref ){
  std::string v1StrL,refStrL;
  std::string v1StrH,refStrH;
  iss >> v1StrL >> v1StrH
      >> refStrL >> refStrH;

  v1=convertStringToRealType(v1StrL, v1StrH);
  ref=convertStringToRealType(refStrL, refStrH);
}

template<class OP,int NBPARAM>
class padRef;

template<class OP>
class padRef<OP,1>{
public:
  template<class REALTYPE>
  static void apply(std::vector<REALTYPE>& refTab){
    refTab.push_back(OP::apply(0.));
  }
};
template<class OP>
class padRef<OP,2>{
public:
  template<class REALTYPE>
  static void apply(std::vector<REALTYPE>& refTab){
    refTab.push_back(OP::apply(0.,1.));
  }
};


//LOAD UCB File for one OP with a specific rounding configuration and check results for scalar, sse and avx
template<class OP>
inline int loadDataAndCheck(const std::string& round){

  typedef typename OP::RealType RealType;
  std::vector<RealType> v1Tab, v2Tab,refTab;
  std::vector<int> lineTab;

  const std::string fileName("./inputData/"+OP::name()+std::string(".input"));

  std::cout <<fileName<<" "<< round <<std::endl;
  //Reference LOADING
  int lineNumber=0;
  std::ifstream input(fileName.c_str());
  std::string line;
  while(std::getline(input,line)){
    lineNumber++;
    if(line[0]=='/'){ //Commentaire
      continue;
    }
    if(line[0]=='\t' && line[1]=='/'){
      continue;
    }

    std::istringstream iss(line);

    std::string fun,dir, eq, sep;
    iss >> fun >> dir >> eq>> sep;
    if(dir!=round){
      continue;
    }
    if(OP::nbArgs==2){
      RealType v1,v2,ref;
      parseEndUCBLine(iss, v1,v2,ref);

      v1Tab.push_back(v1);
      v2Tab.push_back(v2);
      refTab.push_back(ref);
      lineTab.push_back(lineNumber);
    }
    if(OP::nbArgs==1){
      RealType v1,ref;
      parseEndUCBLine_unary(iss, v1,ref);

      if((v1 <0 or v1!=v1) and ( OP::name()==std::string("sqrts") or OP::name()==std::string("sqrtd") )){
	continue;
      }
      v1Tab.push_back(v1);
      refTab.push_back(ref);
      lineTab.push_back(lineNumber);
    }
  }

  //Padding
  while((v1Tab.size() % 8) !=0){
    v1Tab.push_back(0.);
    v2Tab.push_back(1.);
    padRef<OP,OP::nbArgs>::apply(refTab);
    lineTab.push_back(-1);
  }



  int size((int)v1Tab.size());
  bool ok;
  int nbKO=0;

  //Scalar
  std::vector<RealType> testComputationTabScalar(size);
  Loop<OP,RealType,OP::nbArgs>::applyScalar(testComputationTabScalar, v1Tab,v2Tab);
  if(OP::nbArgs==2){
    ok=checkTab(fileName,std::string("Scalar"),testComputationTabScalar,refTab,lineTab,v1Tab,v2Tab);
  }
  if(OP::nbArgs==1){
    ok=checkTab(fileName,std::string("Scalar"),testComputationTabScalar,refTab,lineTab,v1Tab);
  }
  if(!ok) nbKO++;

  //SSE
  std::vector<RealType> testComputationTabSSE(size);
  Loop<OP,RealType,OP::nbArgs>::applySSE(testComputationTabSSE, v1Tab,v2Tab);
  if(OP::nbArgs==2){
    ok=checkTab(fileName,std::string("SSE"),testComputationTabSSE,refTab,lineTab,v1Tab,v2Tab);
  }
  if(OP::nbArgs==1){
    ok=checkTab(fileName,std::string("SSE"),testComputationTabSSE,refTab,lineTab,v1Tab);
  }
  if(!ok) nbKO++;

#ifdef HAVE_AVX
  //AVX
  std::vector<RealType> testComputationTabAVX(size);
  Loop<OP,RealType,OP::nbArgs>::applyAVX(testComputationTabAVX, v1Tab,v2Tab);
  if(OP::nbArgs==2){
    ok=checkTab(fileName,std::string("AVX"),testComputationTabAVX,refTab,lineTab,v1Tab,v2Tab);
  }
  if(OP::nbArgs==1){
    ok=checkTab(fileName,std::string("AVX"),testComputationTabAVX,refTab,lineTab,v1Tab);
  }
#endif

  if(!ok) nbKO++;

  return nbKO;
}



int loadDataAndCheck(std::string rounding){
  int nbKO=0;
  nbKO+=loadDataAndCheck<Add32>(rounding);
  nbKO+=loadDataAndCheck<Add64>(rounding);
  nbKO+=loadDataAndCheck<Mul32>(rounding);
  nbKO+=loadDataAndCheck<Mul64>(rounding);
  nbKO+=loadDataAndCheck<Div32>(rounding);
  nbKO+=loadDataAndCheck<Div64>(rounding);
  nbKO+=loadDataAndCheck<Sub32>(rounding);
  nbKO+=loadDataAndCheck<Sub64>(rounding);

  nbKO+=loadDataAndCheck<Sqrt32>(rounding);
  nbKO+=loadDataAndCheck<Sqrt64>(rounding);

  std::cout << "nbKO:" << nbKO<<std::endl;
  return nbKO;
}



void usage(char** argv){
      std::cout << "usage: "<<argv[0] << " ENV --rounding-mode=[nearest|upward|downward|toward_zero]"<<std::endl;
      std::cout << "ENV is in fenv, valgrind, verificarlo"<<std::endl;
}



int main(int argc, char** argv){
  if(argc!=3){
    usage(argv);
    return EXIT_FAILURE;
  }

  std::string env(argv[1]);
  bool fenv=false;
  if(env==std::string("fenv")){
    fenv=true;
  }else{
    if(env!= std::string("valgrind") && env!=std::string("verificarlo")){
      usage(argv);
      return EXIT_FAILURE;
    }
  }

  std::string option(argv[2]);

  if(option==std::string("--rounding-mode=nearest")){
    if(fenv){
      fesetround(FE_TONEAREST);
    }
    return loadDataAndCheck("n");
  }
  if(option==std::string("--rounding-mode=upward")){
    if(fenv){
      fesetround(FE_UPWARD);
    }
    return loadDataAndCheck("p");
  }
  if(option==std::string("--rounding-mode=downward")){
    if(fenv){
      fesetround(FE_DOWNWARD);
    }
    return loadDataAndCheck("m");
  }
  if(option==std::string("--rounding-mode=toward_zero")){
    if(fenv){
      fesetround(FE_TOWARDZERO);
    }
    return loadDataAndCheck("z");
  }
  usage(argv);
  return EXIT_FAILURE;

}
