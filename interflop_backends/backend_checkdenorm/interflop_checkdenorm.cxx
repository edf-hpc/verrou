
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floating-point operations overloading.         ---*/
/*---                                                 vr_fpOps.cxx ---*/
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

#include "interflop_checkdenorm.h"
#include <stddef.h>
#include <limits>
#include <cmath>

#include <stdarg.h>

#include "../interflop_verrou/vr_fma.hxx"
#include "../interflop_verrou/vr_sqrt.hxx"

checkdenorm_conf_t checkdenorm_conf;


typedef enum check_subnormal_counter {
    CDN_TOTAL=0,
    CDN_OUTPUT,
    CDN_INPUT,
    CDN_INPUT1,
    CDN_INPUT2,
    CDN_INPUT3,
    CDN_COUNTER_SIZE //to count the number of element (always last element of enum)
  } check_subnormal_counter_t;
unsigned long int denorm_counter[CDN_OP_SIZE][CDN_TYPE_SIZE][CDN_COUNTER_SIZE];

const char*  check_denorm_op_name (check_subnormal_op_t op) {
  switch (op) {
  case CDN_ADD:
    return "add";
  case CDN_SUB:
     return "sub";
  case CDN_MUL:
     return "mul";
  case CDN_DIV:
     return "div";
  case CDN_MADD:
     return "mAdd";
  case CDN_SQRT:
     return "sqrt";
  case CDN_CAST:
     return "cast";
  case CDN_OP_SIZE://error (just to avoid warning)
    return "INVALID OP";
  }
  return "undef";
}

unsigned int  check_denorm_nb_args (check_subnormal_op_t op) {
  switch (op) {
  case CDN_ADD:
  case CDN_SUB:
  case CDN_MUL:
  case CDN_DIV:
    return 2;
  case CDN_MADD:
    return 3;
  case CDN_SQRT:
  case CDN_CAST:
    return 1;
  case CDN_OP_SIZE://error (just to avoid warning)
    return 0;
  }
  return 0;
}


const char*  check_denorm_type_name (check_subnormal_type_t type) {
  switch (type) {
  case CDN_DOUBLE:
     return "dbl";
  case CDN_FLOAT:
     return "flt";
  case CDN_TYPE_SIZE://error (just to avoid warning)
    return "INVALID TYPE";
  }
  return "undef";
}


template <typename REAL>
void ifcd_checkdenorm (const REAL & a, const REAL & b, const REAL & r);


// * Global variables & parameters

void (*ifcd_denormInputHandler)(check_subnormal_op_t, check_subnormal_type_t, unsigned int)=NULL;
void (*ifcd_denormOutputHandler)(check_subnormal_op_t, check_subnormal_type_t)=NULL;
void (*ifcd_panicHandler)(const char*)=NULL;

void checkdenorm_set_denorm_input_handler(void (*denormHandler)(check_subnormal_op_t, check_subnormal_type_t, unsigned int)){
  ifcd_denormInputHandler=denormHandler;
}

void checkdenorm_set_denorm_output_handler(void (*denormHandler)(check_subnormal_op_t, check_subnormal_type_t)){
  ifcd_denormOutputHandler=denormHandler;
}

void checkdenorm_set_panic_handler(void (*panicHandler)(const char*)){
  ifcd_panicHandler=panicHandler;
}

void checkdenorm_reset_counter(void){
  for(unsigned int opIndex=0; opIndex< CDN_OP_SIZE; opIndex++){
    for(unsigned int typeIndex=0; typeIndex< CDN_TYPE_SIZE; typeIndex++){
      for(unsigned int counterIndex=0; counterIndex< CDN_COUNTER_SIZE; counterIndex++){
	denorm_counter[opIndex][typeIndex][counterIndex]=0;
      }
    }
  }
}

void checkdenorm_print_counter(myPrintfType myPrintf){
  myPrintf(" ---------------------------------------------------------------------\n");
  myPrintf("Denormalized numbers count\n");
  myPrintf("op-type                %-6s       %-15s     %-15s\n", "Total", "denorm. Input", "denorm. Output");
  for(unsigned int opIndex=0; opIndex< (unsigned int)CDN_OP_SIZE; opIndex++){
    bool opToPrint=false;
    for(unsigned int typeIndex=0; typeIndex < (unsigned int)CDN_TYPE_SIZE; typeIndex++){
      if(denorm_counter[opIndex][typeIndex][CDN_TOTAL]!=0){
	opToPrint=true;
	break;
      }
    }
    if (opToPrint){
      myPrintf(" %s\n", check_denorm_op_name((check_subnormal_op_t)opIndex) );

      for(unsigned int typeIndex=0; typeIndex < (unsigned int)CDN_TYPE_SIZE; typeIndex++){
	if(denorm_counter[opIndex][typeIndex][CDN_TOTAL]!=0){
	  unsigned long int total=denorm_counter[opIndex][typeIndex][CDN_TOTAL];
	  unsigned long int input=denorm_counter[opIndex][typeIndex][CDN_INPUT];
	  unsigned long int output=denorm_counter[opIndex][typeIndex][CDN_OUTPUT];
	  myPrintf("  `-%-6s   %15llu   %12llu(%3u%%)   %12llu(%3u%%)\n",
		   check_denorm_type_name((check_subnormal_type_t)typeIndex),
		   total,
		   input,  (unsigned int)((float)input/(float)total *100.),
		   output, (unsigned int)((float)output/(float)total *100.)
		   );
	}
      }
    }
  }
  myPrintf(" ---------------------------------------------------------------------\n");
}


template<class REAL>
void flushToZeroAndCheck(REAL* res, check_subnormal_type_t etype, check_subnormal_op_t eop){
  if( ( ((*res >= 0) ? (*res): -(*res)))   <  std::numeric_limits<REAL>::min()  && *res !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_OUTPUT]++;
    }
    if(ifcd_denormOutputHandler!=0){
      (*ifcd_denormOutputHandler)(eop,etype);
    }
    if( checkdenorm_conf.flushtozero ){
      *res=0.;
    }
  }
}

template<class REAL>
void denormAreZeroAndCheck(REAL* a, check_subnormal_type_t etype, check_subnormal_op_t eop){
  if(checkdenorm_conf.counter){
    denorm_counter[eop][etype][CDN_TOTAL]++;
  }
  if( ( ((*a >= 0) ? (*a): -(*a)))   <  std::numeric_limits<REAL>::min()  && *a !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT1]++;
      denorm_counter[eop][etype][CDN_INPUT]++;
    }
    if(ifcd_denormInputHandler!=0){
      (*ifcd_denormInputHandler)(eop,etype,1);
    }
    if( checkdenorm_conf.denormarezero){
      *a=0.;
    }
  }
}


template<class REAL>
void denormAreZeroAndCheck(REAL* a, REAL* b, check_subnormal_type_t etype, check_subnormal_op_t eop){
  if(checkdenorm_conf.counter){
    denorm_counter[eop][etype][CDN_TOTAL]++;
  }
  unsigned int nb=0;
  if( ( ((*a >= 0) ? (*a): -(*a)))   <  std::numeric_limits<REAL>::min()  && *a !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT1]++;
    }
    nb++;
    if( checkdenorm_conf.denormarezero){
      *a=0.;
    }
  }
  if( ( ((*b >= 0) ? (*b): -(*b)))   <  std::numeric_limits<REAL>::min()  && *b !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT2]++;
    }
    nb++;
    if( checkdenorm_conf.denormarezero){
      *b=0.;
    }
  }
  if(checkdenorm_conf.counter && nb!=0){
    denorm_counter[eop][etype][CDN_INPUT]++;
  }
  if(ifcd_denormInputHandler!=0 and nb!=0){
    (*ifcd_denormInputHandler)(eop,etype,nb);
  }
}

template<class REAL>
void denormAreZeroAndCheck(REAL* a, REAL* b, REAL* c, check_subnormal_type_t etype, check_subnormal_op_t eop){
  if(checkdenorm_conf.counter){
    denorm_counter[eop][etype][CDN_TOTAL]++;
  }
  unsigned int nb=0;
  if( ( ((*a >= 0) ? (*a): -(*a)))   <  std::numeric_limits<REAL>::min()  && *a !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT1]++;
    }
    nb++;
    if( checkdenorm_conf.denormarezero){
      *a=0.;
    }
  }
  if( ( ((*b >= 0) ? (*b): -(*b)))   <  std::numeric_limits<REAL>::min()  && *b !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT2]++;
    }
    nb++;
    if( checkdenorm_conf.denormarezero){
      *b=0.;
    }
  }
  if( ( ((*c >= 0) ? (*c): -(*c)))   <  std::numeric_limits<REAL>::min()  && *c !=0.){
    if(checkdenorm_conf.counter){
      denorm_counter[eop][etype][CDN_INPUT3]++;
    }
    nb++;
    if( checkdenorm_conf.denormarezero){
      *c=0.;
    }
  }
  if(checkdenorm_conf.counter && nb!=0){
    denorm_counter[eop][etype][CDN_INPUT]++;
  }
  if(ifcd_denormInputHandler!=0 and nb!=0){
    (*ifcd_denormInputHandler)(eop,etype,nb);
  }
}



// * C interface
void IFCD_FCTNAME(configure)(checkdenorm_conf_t mode, void* context) {
  checkdenorm_conf=mode;

  //initialise counter
  if(checkdenorm_conf.counter){
    checkdenorm_reset_counter();
  }
}

void IFCD_FCTNAME(finalize)(void* context){
}

const char* IFCD_FCTNAME(get_backend_name)() {
  return "checkdenorm";
}

const char* IFCD_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

#define APPLYOP(a,b,res,op,realtype, enumType, enumOp) \
  realtype alocal=a;				       \
  realtype blocal=b;				       \
  denormAreZeroAndCheck(&alocal,&blocal,enumType,enumOp);\
  *res=alocal op blocal;			       \
  flushToZeroAndCheck(res,enumType,enumOp);



void IFCD_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,+,double, CDN_DOUBLE, CDN_ADD);
}

void IFCD_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,+,float,CDN_FLOAT, CDN_ADD);
}

void IFCD_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,-,double, CDN_DOUBLE, CDN_SUB);
}

void IFCD_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,-,float, CDN_FLOAT, CDN_SUB);
}


void IFCD_FCTNAME(mul_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,*,double,CDN_DOUBLE, CDN_MUL);
}

void IFCD_FCTNAME(mul_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,*,float,CDN_FLOAT, CDN_MUL);
}


void IFCD_FCTNAME(div_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,/,double,CDN_DOUBLE, CDN_DIV);
}

void IFCD_FCTNAME(div_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,/,float,CDN_FLOAT, CDN_DIV);
}



void IFCD_FCTNAME(madd_float) (float a, float b, float c, float* res,void* context) {
  float alocal=a;
  float blocal=b;
  float clocal=c;
  denormAreZeroAndCheck(&alocal,&blocal,&clocal,CDN_FLOAT,CDN_MADD);
#ifdef USE_VERROU_FMA
  *res=vr_fma(alocal,blocal,clocal);
#else
  ifcd_panicHandler("madd not implemented");
#endif
  flushToZeroAndCheck(res,CDN_FLOAT, CDN_MADD);
}

void IFCD_FCTNAME(madd_double) (double a, double b, double c, double* res,void* context) {
  double alocal=a;
  double blocal=b;
  double clocal=c;
  denormAreZeroAndCheck(&alocal,&blocal,&clocal,CDN_DOUBLE,CDN_MADD);
#ifdef USE_VERROU_FMA
  *res=vr_fma(alocal,blocal,clocal);
#else
  ifcd_panicHandler("madd not implemented");
#endif
  flushToZeroAndCheck(res,CDN_DOUBLE, CDN_MADD);
}

void IFCD_FCTNAME(sqrt_float) (float a, float* res,void* context) {
  float alocal=a;
  denormAreZeroAndCheck(&alocal,CDN_FLOAT,CDN_SQRT);
#ifdef USE_VERROU_SQRT
  *res=vr_sqrt(alocal);
#else
  ifcd_panicHandler("sqrt not implemented");
#endif
  flushToZeroAndCheck(res,CDN_FLOAT, CDN_SQRT);
}


void IFCD_FCTNAME(sqrt_double) (double a, double* res,void* context) {
  double alocal=a;
  denormAreZeroAndCheck(&alocal,CDN_DOUBLE,CDN_SQRT);
#ifdef USE_VERROU_SQRT
  *res=vr_sqrt(alocal);
#else
  ifcd_panicHandler("sqrt not implemented");
#endif
  flushToZeroAndCheck(res,CDN_DOUBLE, CDN_SQRT);
}


void IFCD_FCTNAME(cast_double_to_float) (double a, float* res,void* context) {
  double alocal=a;
  denormAreZeroAndCheck(&alocal,CDN_DOUBLE,CDN_CAST);
  *res=(float)alocal;
  flushToZeroAndCheck(res,CDN_FLOAT, CDN_CAST);
}




struct interflop_backend_interface_t IFCD_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config=interflop_backend_empty_interface;

  config.add_float = & IFCD_FCTNAME(add_float);
  config.sub_float = & IFCD_FCTNAME(sub_float);
  config.mul_float = & IFCD_FCTNAME(mul_float);
  config.div_float = & IFCD_FCTNAME(div_float);

  config.add_double = & IFCD_FCTNAME(add_double);
  config.sub_double = & IFCD_FCTNAME(sub_double);
  config.mul_double = & IFCD_FCTNAME(mul_double);
  config.div_double = & IFCD_FCTNAME(div_double);


  config.cast_double_to_float=& IFCD_FCTNAME(cast_double_to_float);

  config.madd_float = & IFCD_FCTNAME(madd_float);
  config.madd_double =& IFCD_FCTNAME(madd_double);

  config.finalize =& IFCD_FCTNAME(finalize);
  return config;
}
