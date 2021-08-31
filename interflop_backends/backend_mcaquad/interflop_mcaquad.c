
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

#include "interflop_mcaquad.h"
#include <stddef.h>



const char*  mcaquad_mode_name (unsigned int mode) {
  switch (mode) {
  case  MCAMODE_IEEE:
    return "IEEE";
  case  MCAMODE_MCA:
    return "MCA";
  case MCAMODE_PB:
    return "PB";
  case MCAMODE_RR:
    return "RR";
  }
  return "undefined";
}

// * Global variables & parameters
//mcaquad_conf_t mcaquad_conf;
//unsigned int mcaquad_seed;

void (*mcaquad_panicHandler)(const char*)=NULL;

void mcaquad_set_panic_handler(void (*panicHandler)(const char*)){
  mcaquad_panicHandler=panicHandler;
}

void (*mcaquad_debug_print_op)(int,const char*, const double*, const double*)=NULL;
void mcaquad_set_debug_print_op(void (*printOpHandler)(int nbArg,const char*name, const double* args,const double* res)){
  mcaquad_debug_print_op=printOpHandler;
};


#include "mcalib.c"

// * C interface
void IFMQ_FCTNAME(configure)(mcaquad_conf_t mode,void* context) {  
  _set_mca_mode(mode.mode);
  _set_mca_precision(mode.precision_double, mode.precision_float);
}

void IFMQ_FCTNAME(finalize)(void* context){
}

const char* IFMQ_FCTNAME(get_backend_name)() {
  return "mcaquad";
}

const char* IFMQ_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}


static uint64_t mcaquadrandom_seed;

void mcaquad_set_seed (unsigned int seed) {
  uint64_t seed64=(uint64_t) seed;
  _mca_set_seed(&seed64,1);
  mcaquadrandom_seed = tinymt64_generate_uint64(&random_state);
}

void mcaquad_set_random_seed () {
  _mca_set_seed(&mcaquadrandom_seed,1);
}

void IFMQ_FCTNAME(add_double) (double a, double b, double* res,void* context) {
  *res=_mca_dbin(a, b, MCA_ADD);
}

void IFMQ_FCTNAME(add_float) (float a, float b, float* res,void* context) {
  *res=_mca_sbin(a, b, MCA_ADD);
}

void IFMQ_FCTNAME(sub_double) (double a, double b, double* res,void* context) {
  *res=_mca_dbin(a, b, MCA_SUB);
}

void IFMQ_FCTNAME(sub_float) (float a, float b, float* res,void* context) {
  *res=_mca_sbin(a, b, MCA_SUB);
}

void IFMQ_FCTNAME(mul_double) (double a, double b, double* res,void* context) {
  *res=_mca_dbin(a, b, MCA_MUL);
}

void IFMQ_FCTNAME(mul_float) (float a, float b, float* res,void* context) {
  *res=_mca_sbin(a, b, MCA_MUL);
}

void IFMQ_FCTNAME(div_double) (double a, double b, double* res,void* context) {
  *res=_mca_dbin(a, b, MCA_DIV);
}

void IFMQ_FCTNAME(div_float) (float a, float b, float* res,void* context) {
  *res=_mca_sbin(a, b, MCA_DIV);
}

void IFMQ_FCTNAME(cast_double_to_float) (double a, float* res, void* context){
   *res=_mca_dtosbin(a);
}

 void IFMQ_FCTNAME(madd_double) (double a, double b, double c, double* res, void* context){
    *res=_mca_dbin_fma(a,b,c);
 }

void IFMQ_FCTNAME(madd_float) (float a, float b, float c, float* res, void* context){
   *res=_mca_sbin_fma(a,b,c);
 }




struct interflop_backend_interface_t IFMQ_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config= interflop_backend_empty_interface;

  config.add_float = & IFMQ_FCTNAME(add_float);
  config.sub_float = & IFMQ_FCTNAME(sub_float);
  config.mul_float = & IFMQ_FCTNAME(mul_float);
  config.div_float = & IFMQ_FCTNAME(div_float);

  config.add_double = & IFMQ_FCTNAME(add_double);
  config.sub_double = & IFMQ_FCTNAME(sub_double);
  config.mul_double = & IFMQ_FCTNAME(mul_double);
  config.div_double = & IFMQ_FCTNAME(div_double);

  config.cast_double_to_float= & IFMQ_FCTNAME(cast_double_to_float);

  config.madd_float = & IFMQ_FCTNAME(madd_float);
  config.madd_double = & IFMQ_FCTNAME(madd_double);

  config.finalize=& IFMQ_FCTNAME(finalize);
  return config;
}
