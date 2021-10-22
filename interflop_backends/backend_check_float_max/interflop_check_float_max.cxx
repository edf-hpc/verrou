
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floating-point operations overloading.         ---*/
/*---                                  interflop_check_float_max.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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

#include "interflop_check_float_max.h"
#include <stddef.h>
#include <float.h>

//check_float_max_conf_t check_float_max_conf;

template <typename REAL>
void ifmax_checkMax (const REAL & a, const REAL & b, const REAL & r);


// * Global variables & parameters

void (*ifmax_maxHandler)(void)=NULL;

void ifmax_set_max_handler(void (*maxHandler)(void)){
  ifmax_maxHandler=maxHandler;
}

void (*ifmax_debug_print_op)(int,const char*, const double*, const double*)=NULL;
void ifmax_set_debug_print_op(void (*printOpHandler)(int nbArg,const char*name, const double* args,const double* res)){
  ifmax_debug_print_op=printOpHandler;
};



template<typename REAL>
bool ifmax_is_flt_max(const REAL& a);

template<>
bool ifmax_is_flt_max(const float& a){
  return a==FLT_MAX;
}
template<>
bool ifmax_is_flt_max(const double& a){
  return a==DBL_MAX;
}


template <typename REAL>
inline
void ifmax_checkmax (const char* op, const REAL & a, const REAL & b, const REAL & r) {

  if (ifmax_is_flt_max(a) || ifmax_is_flt_max(b)) {
    if(ifmax_debug_print_op!=NULL){
      double param[2]={a,b};
      double res[1]={r};
      ifmax_debug_print_op(2,op,param,res);
    }
    ifmax_maxHandler();
  }
}

template <typename REAL>
inline
void ifmax_checkmax (const char* op, const REAL & a, const REAL & b,  const REAL & c, const REAL & r) {

  if (ifmax_is_flt_max(a) || ifmax_is_flt_max(b) || ifmax_is_flt_max(c) ) {
    if(ifmax_debug_print_op!=NULL){
      double param[3]={a,b,c};
      double res[1]={r};
      ifmax_debug_print_op(3,op,param,res);
    }
    ifmax_maxHandler();
  }
}


template <typename REAL>
inline
void ifmax_checkmax (const char* op, const REAL & a, const float & r) {

  if (ifmax_is_flt_max(a) || ifmax_is_flt_max(r)) {
    if(ifmax_debug_print_op!=NULL){
      double param[1]={a};
      double res[1]={r};
      ifmax_debug_print_op(1,op,param,res);
    }
    ifmax_maxHandler();
  }
}



// * C interface
//void IFMAX_FCTNAME(configure)(check_float_max_conf_t mode, void* context) {
//  check_float_max_conf=mode;
//}

void IFMAX_FCTNAME(finalize)(void* context){
}

const char* IFMAX_FCTNAME(get_backend_name)() {
  return "check_float_max";
}

const char* IFMAX_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

void IFMAX_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  ifmax_checkmax("add_double",a,b,*res);
}

void IFMAX_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  ifmax_checkmax("add_float",a,b,*res);
}

void IFMAX_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  ifmax_checkmax("sub_double",a,b,*res);
}

void IFMAX_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  ifmax_checkmax("sub_float",a,b,*res);
}


void IFMAX_FCTNAME(mul_double) (double a, double b, double* res,void* context) {
  ifmax_checkmax("mul_double",a,b,*res);
}

void IFMAX_FCTNAME(mul_float) (float a, float b, float* res,void* context) {
  ifmax_checkmax("mul_float",a,b,*res);
}

void IFMAX_FCTNAME(div_double) (double a, double b, double* res,void* context) {
  ifmax_checkmax("div_double",a,b,*res);
}

void IFMAX_FCTNAME(div_float) (float a, float b, float* res,void* context) {
  ifmax_checkmax("div_float",a,b,*res);
}
void IFMAX_FCTNAME(cast_double_to_float) (double a, float* res,void* context) {
  ifmax_checkmax("cast_double_to_float",a,*res);
}

void IFMAX_FCTNAME(madd_double) (double a, double b, double c, double* res, void* context){
  ifmax_checkmax("madd_double",a,b,c,*res);
}

void IFMAX_FCTNAME(madd_float) (float a, float b, float c, float* res, void* context){
  ifmax_checkmax("madd_float",a,b,c,*res);
}




struct interflop_backend_interface_t IFMAX_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config=interflop_backend_empty_interface;

  config.add_float = & IFMAX_FCTNAME(add_float);
  config.sub_float = & IFMAX_FCTNAME(sub_float);
  config.mul_float = & IFMAX_FCTNAME(mul_float);;
  config.div_float = & IFMAX_FCTNAME(div_float);;

  config.add_double = & IFMAX_FCTNAME(add_double);
  config.sub_double = & IFMAX_FCTNAME(sub_double);
  config.mul_double = & IFMAX_FCTNAME(mul_double);;
  config.div_double = & IFMAX_FCTNAME(div_double);;


  config.cast_double_to_float=& IFMAX_FCTNAME(cast_double_to_float);

  config.madd_float = & IFMAX_FCTNAME(madd_float);
  config.madd_double =& IFMAX_FCTNAME(madd_double);

  config.finalize = & IFMAX_FCTNAME(finalize);

  return config;
}
