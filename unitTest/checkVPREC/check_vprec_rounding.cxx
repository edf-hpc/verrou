#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "verrou.h"


template<class REALTYPE>
REALTYPE perform_op(char op, REALTYPE* tab) {
  VERROU_START_INSTRUMENTATION;
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
  VERROU_STOP_INSTRUMENTATION;
  return res;
}

bool isValidOp(char op){
   switch(op){
   case '+':
   case '-':
   case 'x':
   case '/':
      return true;
   default:
      return false;
   }
}


bool parseLine(char* line,char* op, float* argsOp, float* ref){
// + +0x1.3be88f5a8c2b8p-2 -0x1.2b46c18de74dcp-3 => +0x1.4c8a5d2731090p-3
   char* cur=line;
   if(!isValidOp(cur[0])){
      fprintf(stderr, "bad format invalid op\n");
      fprintf(stderr, "bad format %s\n", line);
      exit(EXIT_FAILURE);
   }
   *op=cur[0];

   cur=line+2;
   argsOp[0]=strtof(cur, &cur );
   argsOp[1]=strtof(cur, &cur );

   cur=cur+4;

   *ref=strtof(cur, &cur );

   return true;
}


bool parseLine(char* line,char* op, double* argsOp, double* ref){
// + +0x1.3be88f5a8c2b8p-2 -0x1.2b46c18de74dcp-3 => +0x1.4c8a5d2731090p-3
   char* cur=line;
   if(!isValidOp(cur[0])){
      fprintf(stderr, "bad format invalid op\n");
      fprintf(stderr, "bad format %s\n", line);
      exit(EXIT_FAILURE);
   }
   *op=cur[0];

   cur=line+2;
   argsOp[0]=strtod(cur, &cur );
   argsOp[1]=strtod(cur, &cur );

   cur=cur+4;

   *ref=strtod(cur, &cur );

   return true;
}



void printError(float ref, float a, float relError){
  printf("ref : %+.6a\t%.8e\n", ref,ref);
  printf("vprec : %+.6a\t%.8e\n", a,a);
  printf("rel : %+.6a\t%.8e\n", relError, relError);
};

void printError(double ref, double a, double relError){
  printf("ref : %+.13a\t%.17e\n", ref,ref);
  printf("vprec : %+.13a\t%.17e\n", a,a);
  printf("rel : %+.13a\t%.17e\n", relError, relError);
};

template<class REALTYPE>
bool cmpFaithFulFloat(REALTYPE ref, REALTYPE a, int range, int precision){
  if ( ref == a){
    return true;
  }
  if( ref!=ref && a!=a){ //NaN
    return true;
  }

  REALTYPE relError=abs((ref -a) / ref) ;
   if( relError  * pow(2,precision-1) <= 1){
      return true;
   }else{
     printError(ref,a,relError);
     return false;
   }
}




int main(int argc, char * argv[]) {
  VERROU_STOP_INSTRUMENTATION;
  if (argc != 4) {
    fprintf(stderr, "3 arguments expected : referenceFile EXPOSANT MANTISSE\n");
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


  char line[512];
  int counterOK=0;
  int counterKO=0;
  char op;

  if( floatFile){
    while(fgets(line, 512, refFile) ){
      float argsOp[2];
      float ref;

      parseLine(line,&op, argsOp, &ref);
      float vprecRes=perform_op(op,argsOp);

      if(cmpFaithFullFloat(ref, vprecRes, exposant, mantisse)){
	counterOK++;
      }else{
	counterKO++;
	printf("Error : resVprec [%+.6a] != ref [%+.6a] \t refLine:%s\n", vprecRes, ref, line);
      }
     }
  }else{//double
     while(fgets(line, 512, refFile) ){
       double argsOp[2];
       double ref;
       parseLine(line,&op, argsOp, &ref);
       double vprecRes=perform_op(op,argsOp);

       if(cmpFaithFullFloat(ref, vprecRes, exposant, mantisse)){
	  counterOK++;
       }else{
	 counterKO++;
	 printf("Error : resVprec [%+.13a] != ref [%+.13a] \t refLine:%s\n", vprecRes, ref, line);
       }
     }
  }

  printf("OK: %d \n", counterOK);
  printf("KO: %d \n", counterKO);
  if(counterKO!=0){
     return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
