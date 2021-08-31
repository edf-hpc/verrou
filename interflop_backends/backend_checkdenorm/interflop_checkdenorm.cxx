
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

#include "../interflop_verrou/vr_fma.hxx"

checkdenorm_conf_t checkdenorm_conf;

template <typename REAL>
void ifcd_checkdenorm (const REAL & a, const REAL & b, const REAL & r);


// * Global variables & parameters

void (*ifcd_denormHandler)(void)=NULL;
void (*ifcd_panicHandler)(const char*)=NULL;

void checkdenorm_set_denorm_handler(void (*denormHandler)(void)){
  ifcd_denormHandler=denormHandler;
}

void checkdenorm_set_panic_handler(void (*panicHandler)(const char*)){
  ifcd_panicHandler=panicHandler;
}



template<class REAL>
void flushToZeroAndCheck(REAL* res){
  if( std::abs(*res) <  std::numeric_limits<REAL>::min()  && *res !=0.){
    if(ifcd_denormHandler!=0){
      (*ifcd_denormHandler)();
    }
    if( checkdenorm_conf.flushtozero ){
      *res=0.;
    }
    
  }
}



// * C interface
void IFCD_FCTNAME(configure)(checkdenorm_conf_t mode, void* context) {
  checkdenorm_conf=mode;
}

void IFCD_FCTNAME(finalize)(void* context){
}

const char* IFCD_FCTNAME(get_backend_name)() {
  return "checkdenorm";
}

const char* IFCD_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

#ifdef IFCD_DOOP
#define APPLYOP(a,b,res,op)\
  *res=a op b;\
  flushToZeroAndCheck(res);
#else
#define APPLYOP(a,b,res,op)\
  flushToZeroAndCheck(res);
#endif



void IFCD_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,+);
}

void IFCD_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,+);
}

void IFCD_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,-);
}

void IFCD_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,-);
}


void IFCD_FCTNAME(mul_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,*);
}

void IFCD_FCTNAME(mul_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,*);
}


void IFCD_FCTNAME(div_double) (double a, double b, double* res,void* context) {
  APPLYOP(a,b,res,/);
}

void IFCD_FCTNAME(div_float) (float a, float b, float* res,void* context) {
  APPLYOP(a,b,res,/);
}



void IFCD_FCTNAME(madd_float) (float a, float b, float c, float* res,void* context) {
#ifdef IFCD_DOOP
#ifdef USE_VERROU_FMA
  *res=vr_fma(a,b,c);
#else
  ifcd_panicHandler("madd not implemented");
#endif
#endif
  flushToZeroAndCheck(res);
}

void IFCD_FCTNAME(madd_double) (double a, double b, double c, double* res,void* context) {
#ifdef IFCD_DOOP
#ifdef USE_VERROU_FMA
  *res=vr_fma(a,b,c);
#else
  ifcd_panicHandler("madd not implemented");
#endif
#endif
  flushToZeroAndCheck(res);
}


void IFCD_FCTNAME(cast_double_to_float) (double a, float* res,void* context) {
#ifdef  IFCD_DOOP
  *res=(float)a;
#endif
  flushToZeroAndCheck(res);
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
