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


#define USE_VERROU_FMA
#include "../../interflop_backends/interflop_verrou/vr_fma.hxx"

#define USE_VERROU_SQRT
#include "../../interflop_backends/interflop_verrou/vr_sqrt.hxx"


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
    res = vr_fma<REALTYPE>(a,b,c);
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
    res = vr_sqrt<REALTYPE>(a);
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


template<class REALTYPE>
REALTYPE strToRealtype(char* str, char** next);

template<>
double strToRealtype<double>(char* str, char** next){
  double res=strtod(str, next );
  return res;
}

template<>
float strToRealtype<float>(char* str, char** next){
  float res=strtof(str, next );
  return res;
}


template<class REALTYPE>
bool parseLine(char* line, bool* exact, char* op, REALTYPE* argsOp, REALTYPE* ref){
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
     argsOp[i]=strToRealtype<REALTYPE>(cur, &cur);
   }
   cur=cur+4;

   *ref=strToRealtype<REALTYPE>(cur, &cur);
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
  equal_ulp=1,
  inf_ulp=2,
  inf_2ulp=3,
  sup_2ulp=4
} cmp_res_t;

template<class REALTYPE>
cmp_res_t cmpFaithFulFloat(REALTYPE ref, REALTYPE a, int range, int precision){
  if ( ref == a){
    return equal_exact;
  }
  if( isnan(ref) && isnan(a)){ //NaN
    return equal_exact;
  }
  if(ref==0){
    printf("0 expected but %.17e\t%a\n",a ,a);
    return sup_2ulp;
  }

  REALTYPE absError=std::abs((ref -a)) ;
  REALTYPE relError=std::abs((ref -a) / ref) ;
  printf("absError: %.17e\t relError %.17e\n" , absError, relError);
  char normalStr[]="normal";
  char subnormalStr[]="subnormal";
  char* normalityStatusStr=normalStr;
  if( fabs(ref)>= floatMinNorm(range,precision)){
    normalityStatusStr=subnormalStr;
  }

  if( absError == getUlp(ref, range, precision)){
    printf("=+ulp OK (%s)\n",normalityStatusStr);
    printError(ref,a,relError,absError,range,precision);
    return equal_ulp;
  }
  else if(  absError <= getUlp(ref, range, precision)){
    printf("<ulp (%s)\n",normalityStatusStr);
    printError(ref,a,relError,absError,range,precision);
    return inf_ulp;
  }
  else if( absError <= 2.*getUlp(ref, range, precision)){
    printf("<2ulp (%s)\n", normalityStatusStr);
    printError(ref,a,relError,absError,range,precision);
    return inf_2ulp;
  } else{
    printf("fail (%s)\n",normalityStatusStr);
    printError(ref,a,relError,absError,range,precision);
    return sup_2ulp;
  }
  return sup_2ulp;
}

template<class REALTYPE>
struct ErrorLineFormat;

template<>
struct ErrorLineFormat<float>{
  static const char* getStr(){
    return "Error : resVprec [%+.6a] != ref [%+.6a] \t refLine:%s\n";
  }
};
template<>
struct ErrorLineFormat<double>{
  static const char* getStr(){
    return "Error : resVprec [%+.13a] != ref [%+.13a] \t refLine:%s\n";
  }
};


template<class REALTYPE>
int loopOverReferenceFile(char* fileName, int nbSampleMax, int range, int precision,
			  bool isIB, bool isOB, bool isFULL){
  FILE* refFile=fopen(fileName,"r");
  if(refFile==NULL){
    printf("file %s impossible to open\n", fileName);
    return EXIT_FAILURE;
  }
  char line[512];
  int counterOK=0;
  int counterOKTol=0;
  int counterKO=0;
  int count=0;

  while(fgets(line, 512, refFile) ){
    REALTYPE argsOp[3];
    REALTYPE ref=1.;
    bool exact=false;
    char op;
    parseLine<REALTYPE>(line,&exact, &op, argsOp, &ref);

    REALTYPE vprecRes=perform_op(op,argsOp);

    cmp_res_t cmpRes=cmpFaithFulFloat<REALTYPE>(ref, vprecRes, range, precision);

    if(exact){
      if(cmpRes==equal_exact){
	counterOK++;
      }else{
	counterKO++;
	printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
      }
    }else{//mid point detected in reference
      switch(cmpRes){
      case equal_exact:
	counterOK++;
	break;
      case equal_ulp:
	counterOKTol++;
	printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
	break;
      case inf_ulp:
	if(isOB){
	  counterKO++;
	  printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
	}
	if(isIB || isFULL){
	  counterOKTol++;
	  printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
	}
	break;

      case inf_2ulp:
      case sup_2ulp:
	if(isOB){
	  counterKO++;
	  printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
	}
	if(isIB || isFULL){
	  counterKO++;
	  printf("No necessary an error, but I need a case to analyse\n");
	  printf(ErrorLineFormat<REALTYPE>::getStr(), vprecRes, ref, line);
	}
	break;
      default:
	printf("error switch\n");
      }
    }
    count++;
    if(count == nbSampleMax){
      break;
    }
  }

  printf("OK: %d \n", counterOK);
  printf("OK(tol): %d \n", counterOKTol);
  printf("KO: %d \n", counterKO);
  if(counterKO!=0){
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
};







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
     printf("impossible to detect float or double type\n");
     return EXIT_FAILURE;
  }


  bool fullFile=false;
  if( strstr(argv[1], "full")!=NULL){
     fullFile=true;
     printf("full detected\n");
  }
  bool obFile=false;
  if( strstr(argv[1], "ob")!=NULL){
     obFile=true;
     printf("ob detected\n");
  }
  bool ibFile=false;
  if( strstr(argv[1], "ib")!=NULL){
     ibFile=true;
     printf("ib detected\n");
  }

  if( ! ((fullFile ^ obFile) || (fullFile ^ ibFile) || (ibFile ^ obFile) ) ){
     printf("impossible to detect full/ib/ob simultaneously\n");
     return EXIT_FAILURE;
  }
  int range=atoi(argv[2]);
  int precision=atoi(argv[3]);
  printf("range %d\tprecision %d\n", range, precision);

  int nbSampleMax=INT_MAX;
  if( argc==5){
    nbSampleMax=atoi(argv[4]);
  }

  if(floatFile){
    return loopOverReferenceFile<float>(argv[1], nbSampleMax, range, precision, ibFile, obFile, fullFile);
  }

  if(doubleFile){
    return loopOverReferenceFile<double>(argv[1],nbSampleMax, range, precision, ibFile, obFile, fullFile);
  }
  return EXIT_FAILURE;

}
