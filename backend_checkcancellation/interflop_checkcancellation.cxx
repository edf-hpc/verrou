
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floating-point operations overloading.         ---*/
/*---                                                 vr_fpOps.cxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2016
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "interflop_checkcancellation.h"
#include "vr_fpRepr.hxx"
#include <stddef.h>




template <typename REAL>
void vr_checkCancellation (const REAL & a, const REAL & b, const REAL & r);


// * Global variables & parameters

void (*vrcc_cancellationHandler)(int)=NULL;
void (*vrcc_panicHandler)(const char*)=NULL;

void checkcancellation_set_cancellation_handler(void (*cancellationHandler)(int)){
  vrcc_cancellationHandler=cancellationHandler;
}

void checkcancellation_set_panic_handler(void (*panicHandler)(const char*)){
  vrcc_panicHandler=panicHandler;
}




template <typename REAL>
inline
void vr_checkCancellation (const REAL & a, const REAL & b, const REAL & r) {

  const int ea = exponentField (a);
  const int eb = exponentField (b);
  const int er = exponentField (r);

  const int emax = ea>eb ? ea : eb;
  const int cancelled = emax - er;

  if (cancelled >= storedBits(a)) {
    vrcc_cancellationHandler(cancelled);
  }
}



// * C interface
void IFCC_FCTNAME(configure)(void* mode, void* context) {

}

void IFCC_FCTNAME(finalyze)(void* context){
}

const char* IFCC_FCTNAME(get_backend_name)() {
  return "checkcancellation";
}

const char* IFCC_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

void IFCC_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  vr_checkCancellation(a,b,*res);
}

void IFCC_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  vr_checkCancellation(a,b,*res);
}

void IFCC_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  vr_checkCancellation(a,b,*res);
}

void IFCC_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  vr_checkCancellation(a,b,*res);
}


void IFCC_FCTNAME(madd_double) (double a, double b, double c, double* res, void* context){
  vr_checkCancellation(a*b,c,*res);
}

void IFCC_FCTNAME(madd_float) (float a, float b, float c, float* res, void* context){
  vr_checkCancellation(a*b,c,*res);
}




struct interflop_backend_interface_t IFCC_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config;

  config.interflop_add_float = & IFCC_FCTNAME(add_float);
  config.interflop_sub_float = & IFCC_FCTNAME(sub_float);
  config.interflop_mul_float = NULL;
  config.interflop_div_float = NULL;

  config.interflop_add_double = & IFCC_FCTNAME(add_double);
  config.interflop_sub_double = & IFCC_FCTNAME(sub_double);
  config.interflop_mul_double = NULL;
  config.interflop_div_double = NULL;

  config.interflop_cast_double_to_float=NULL;

  config.interflop_madd_float = & IFCC_FCTNAME(madd_float);
  config.interflop_madd_double =& IFCC_FCTNAME(madd_double);

  return config;
}
