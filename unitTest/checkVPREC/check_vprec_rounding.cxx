#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include <climits>
#include "verrou.h"

#include "fpPropImpl.c"

#if defined(__x86_64__)
#include  <immintrin.h>

inline double mySqrt(const double& a){
    double d;
    __m128d ai,di;
    ai = _mm_load_sd(&a);
    di=_mm_sqrt_sd(ai,ai);
    d=_mm_cvtsd_f64(di);
    return d;
  }


  inline float mySqrt(const float& a){
    float d;
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    di=_mm_sqrt_ss(ai);
    d=_mm_cvtss_f32(di);

    return d;
  }
#elif defined(__aarch64__)
#include "arm_neon.h"


inline double mySqrt(const double& a){
  const float64x1_t ap=vld1_f64(&a);
  const float64x1_t res_p=vsqrt_f64(ap);
  double res;
  vst1_f64(&res,res_p);
  return res;
}


inline float vrSqrt(const float& a){

  float av[2]={a,0};
  float32x2_t ap=vld1_f32(av);
  float32x2_t resp= vsqrt_f32(ap);
  float res[2];
  vst1_f32(res, resp);
  return res[0];
}

#else
template<class REALTYPE>
inline REALTYPE mySqrt(REALTYPE a);

template<>
inline double mySqrt<double>(double a){
  return __builtin_sqrt(a);
}
template<>
inline float mySqrt<float>(float a){
  return __builtin_sqrtf(a);
}
#endif


template<class REALTYPE>
REALTYPE perform_op_2(char op, REALTYPE* tab) {
  VERROU_START_SOFT_INSTRUMENTATION;
  REALTYPE a=tab[0];
  REALTYPE b=tab[1];
  REALTYPE res;
  switch(op){
  case '+':
    res = (a) + (b);
    break;
  case '-':
    res = (a) - (b);
    break;
  case 'x':
    res= (a) * (b);
    break;
  case '/':
    res = (a) / (b);
    break;
  default:
    fprintf(stderr, "Bad op %c\n",op);
    exit(EXIT_FAILURE);
  }
  VERROU_STOP_SOFT_INSTRUMENTATION;
  return res;
}

template<class REALTYPE>
REALTYPE perform_op_3(char op, REALTYPE* tab) {
  VERROU_START_SOFT_INSTRUMENTATION;
  REALTYPE a=tab[0];
  REALTYPE b=tab[1];
  REALTYPE c=tab[2];
  REALTYPE res;
  switch(op){
  case 'f':
    res = __builtin_fma(a,b,c);
    break;
  default:
    fprintf(stderr, "Bad op %c\n",op);
    exit(EXIT_FAILURE);
  }
  VERROU_STOP_SOFT_INSTRUMENTATION;
  return res;
}

template<class REALTYPE>
REALTYPE perform_op_1(char op, REALTYPE* tab) {
  VERROU_START_SOFT_INSTRUMENTATION;
  REALTYPE a=tab[0];
  REALTYPE res;
  switch(op){
  case 's':
    res = mySqrt(a);
    break;
  default:
    fprintf(stderr, "Bad op %c\n",op);
    exit(EXIT_FAILURE);
  }
  VERROU_STOP_SOFT_INSTRUMENTATION;
  return res;
}


int opToNbArgs(char op){
   switch(op){
   case '+':
   case '-':
   case 'x':
   case '/':
     return 2;
   case 's':
     return 1;
   case 'f':
      return 3;
   default:
      return -1;
   }
}

template<class REALTYPE>
REALTYPE perform_op(char op, REALTYPE* tab) {
  int nbArg=opToNbArgs(op);
  if(nbArg==1) return perform_op_1(op,tab);
  if(nbArg==2) return perform_op_2(op,tab);
  if(nbArg==3) return perform_op_3(op,tab);
  fprintf(stderr, "Bad op %c\n",op);
  exit(EXIT_FAILURE);
}


bool parseLine(char* line, bool* exact, char* op, float* argsOp, float* ref){
//= + +0x1.3be88f5a8c2b8p-2 -0x1.2b46c18de74dcp-3 => +0x1.4c8a5d2731090p-3
   char* cur=line;
   char exactOrApprox=cur[0];
   if(exactOrApprox=='='){
     *exact=true;
   }else{
     if(exactOrApprox=='~'){
       *exact=false;
     }else{
       fprintf(stderr, "bad format %s\n", line);
       exit(EXIT_FAILURE);
     }
   }
   cur=cur+2;
   int nbArgs=opToNbArgs(cur[0]);
   if(nbArgs==-1){
      fprintf(stderr, "bad format invalid op\n");
      fprintf(stderr, "bad format %s\n", line);
      exit(EXIT_FAILURE);
   }
   *op=cur[0];
   cur=cur+2;

   for(int i=0; i< nbArgs; i++){
     argsOp[i]=strtof(cur, &cur );
   }
   cur=cur+4;

   *ref=strtof(cur, &cur );
   return true;
}

bool parseLine(char* line, bool* exact, char* op, double* argsOp, double* ref){
// + +0x1.3be88f5a8c2b8p-2 -0x1.2b46c18de74dcp-3 => +0x1.4c8a5d2731090p-3
   char* cur=line;
   char exactOrApprox=cur[0];
   if(exactOrApprox=='='){
     *exact=true;
   }else{
     if(exactOrApprox=='~'){
       *exact=false;
     }else{
       fprintf(stderr, "bad format %s\n", line);
       exit(EXIT_FAILURE);
     }
   }
   cur=cur+2;

   int nbArgs=opToNbArgs(cur[0]);
   if(nbArgs==-1){
      fprintf(stderr, "bad format invalid op\n");
      fprintf(stderr, "bad format %s\n", line);
      exit(EXIT_FAILURE);
   }
   *op=cur[0];
   cur=cur+2;

   for(int i=0; i< nbArgs; i++){
     argsOp[i]=strtod(cur, &cur );
   }
   cur=cur+4;

   *ref=strtod(cur, &cur );
   return true;
}



void printError(float ref, float a, float relError, float absError, int range, int precision){
  printf("ref : %+.6a\t%.8e\n", ref,ref);
  printf("vprec : %+.6a\t%.8e\n", a,a);
  printf("rel : %+.6a\t%.8e\t%.2fulp\n", relError, relError, absError / getUlp(ref, range, precision));
  printf("abs error : %+.6a\t%.8e\n", absError, absError);
};


void printError(double ref, double a, double relError,double absError, int range, int precision){
  printf("ref : %+.13a\t%.17e\n", ref,ref);
  printf("vprec : %+.13a\t%.17e\n", a,a);
  printf("rel : %+.13a\t%.17e\t%.2fulp\n", relError, relError, absError / getUlp(ref, range, precision));
  printf("abs error : %+.13a\t%.17e\n", absError, absError);

};



typedef enum {
  equal_exact=0,
  equal_tol=1,
  diff=2
} cmp_res_t;

template<class REALTYPE>
cmp_res_t cmpFaithFulFloat(REALTYPE ref, REALTYPE a, int range, int precision){
  if ( ref == a){
    return equal_exact;
  }
  if( ref!=ref && a!=a){ //NaN
    return equal_exact;
  }
  if(ref==0){
    printf("0 expected but %.17e\t%a\n",a ,a);
    return diff;
  }

  REALTYPE error=abs((ref -a)) ;
  REALTYPE relError=abs((ref -a) / ref) ;

  if( abs(ref)> floatMinNorm(range,precision)){
    if( relError  * pow(2,precision) <= 1){
      printf("tol OK (normal)\n");
      printError(ref,a,relError,error,range,precision);
      return equal_tol;
    }else{
      printf("fail (normal)\n");
      printError(ref,a,relError,error,range,precision);
      return diff;
    }
  }else{
    if( error <= getUlp(ref, range, precision)){
      printf("tol OK (denormal)\n");
      printError(ref,a,relError,error,range,precision);
      return equal_tol;
    }else{
      printf("fail (denormal)\n");
      printError(ref,a,relError,error,range,precision);
      return diff;
    }
  }
}




int main(int argc, char * argv[]) {
  VERROU_STOP_SOFT_INSTRUMENTATION;
  if (! ((argc == 4) || (argc == 5))) {
    fprintf(stderr, "3 arguments expected : referenceFile EXPOSANT MANTISSE [#max_sample]\n");
    exit(EXIT_FAILURE);
  }

  bool floatFile=false;
  if( strstr(argv[1], "float")!=NULL){
     floatFile=true;
     printf("float detected\n");
  }
  bool doubleFile=false;
  if( strstr(argv[1], "double")!=NULL){
     doubleFile=true;
     printf("double detected\n");
  }

  if( ! (floatFile ^ doubleFile)){
     printf("impossible to detect float or double type");
     return EXIT_FAILURE;
  }

  FILE* refFile=fopen(argv[1],"r");
  if(refFile==NULL){
    printf("file %s impossible to open\n", argv[1]);
    return EXIT_FAILURE;
  }

  int exposant;
  int mantisse;
  exposant=atoi(argv[2]);
  mantisse=atoi(argv[3]);
  printf("exposant %d mantisse %d\n", exposant, mantisse);

  int nbSample=INT_MAX;
  if( argc==5){
    nbSample=atoi(argv[4]);
  }

  char line[512];
  int counterOK=0;
  int counterOKTol=0;
  int counterKO=0;
  char op;
  int count=0;
  if( floatFile){
    while(fgets(line, 512, refFile) ){
      float argsOp[3];
      float ref;
      bool exact;

      parseLine(line,&exact, &op, argsOp, &ref);
      float vprecRes=perform_op(op,argsOp);

      cmp_res_t res=cmpFaithFulFloat(ref, vprecRes, exposant, mantisse);
      if(res==equal_exact){
	counterOK++;
      }else if(res==equal_tol){
	if(exact){
	  counterKO++;
	  printf("Error : resVprec [%+.6a] != ref [%+.6a] \t refLine:%s\n", vprecRes, ref, line);
	  printf("args %.8e %.8e\n", argsOp[0],argsOp[1]);
	}else{
	  counterOKTol++;
	  printf("Error : resVprec [%+.6a] != ref [%+.6a] \t refLine:%s\n", vprecRes, ref, line);
	}
      }
      else{
	counterKO++;
	printf("Error : resVprec [%+.6a] != ref [%+.6a] \t refLine:%s\n", vprecRes, ref, line);
	printf("args %.8e %.8e\n", argsOp[0],argsOp[1]);
      }
      count++;
      if(count == nbSample){
	break;
      }
     }
  }else{//double
     while(fgets(line, 512, refFile) ){
       double argsOp[3];
       double ref;
       bool exact;

       parseLine(line,&exact, &op, argsOp, &ref);
       double vprecRes=perform_op(op,argsOp);
       cmp_res_t res=cmpFaithFulFloat(ref, vprecRes, exposant, mantisse);
       if(res==equal_exact){
	 counterOK++;
       }else if(res==equal_tol){
	 if(exact){
	   counterKO++;
	   printf("Error : resVprec [%+.13a] != ref [%+.13a] \t refLine:%s\n", vprecRes, ref, line);
	 }else{
	   counterOKTol++;
	   printf("Error : resVprec [%+.13a] != ref [%+.13a] \t refLine:%s\n", vprecRes, ref, line);
	 }
       }else{
	 counterKO++;
	 printf("Error : resVprec [%+.13a] != ref [%+.13a] \t refLine:%s\n", vprecRes, ref, line);
       }
       count++;
       if(count == nbSample){
	 break;
       }
     }
  }

  printf("OK: %d \n", counterOK);
  printf("OK(tol): %d \n", counterOKTol);
  printf("KO: %d \n", counterKO);
  if(counterKO!=0){
     return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
