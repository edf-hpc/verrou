
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floatin-point operations overloading.          ---*/
/*---                                                   vr_fpops.h ---*/
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

#ifndef __INTERFLOP_MCAQUAD_H
#define __INTERFLOP_MCAQUAD_H

//#define DEBUG_PRINT_OP

#ifdef __cplusplus
extern "C" {
#endif
#define IFMQ_FCTNAME(FCT) interflop_mcaquad_##FCT

#include "../interflop.h"


   /* Repris depuis vfcwrapper.h*/
   /* define the available MCA modes of operation */
#define MCAMODE_IEEE 0
#define MCAMODE_MCA  1
#define MCAMODE_PB   2
#define MCAMODE_RR   3
   /* Fin Reprise depuis vfcwrapper.h*/

  const char*  mcaquad_mode_name (unsigned int mode);
   
  struct mcaquad_conf {
     unsigned int precision_float;
     unsigned int precision_double;
     int mode;
  };
   
  typedef struct mcaquad_conf mcaquad_conf_t;

  void IFMQ_FCTNAME(configure)(mcaquad_conf_t mode,void* context);
  void IFMQ_FCTNAME(finalize)(void* context);

  const char* IFMQ_FCTNAME(get_backend_name)(void);
  const char* IFMQ_FCTNAME(get_backend_version)(void);


//  const char* verrou_rounding_mode_name (enum vr_RoundingMode mode);

   //void verrou_begin_instr(void);
   //void verrou_end_instr(void);

  void mcaquad_set_seed (unsigned int seed);
  void mcaquad_set_random_seed (void);

   //void verrou_set_cancellation_handler(void (*)(int));

  extern void (*mcaquad_panicHandler)(const char*);
  void mcaquad_set_panic_handler(void (*)(const char*));

  extern void (*mcaquad_debug_print_op)(int,const char*, const double* args, const double* res);
  void mcaquad_set_debug_print_op(void (*)(int nbArg, const char* name, const double* args, const double* res));

  struct interflop_backend_interface_t IFMQ_FCTNAME(init)(void ** context);

  void IFMQ_FCTNAME(add_double) (double a, double b, double* res, void* context);    
  void IFMQ_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFMQ_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFMQ_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFMQ_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFMQ_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFMQ_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFMQ_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFMQ_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFMQ_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFMQ_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);

  
#ifdef __cplusplus
}
#endif

#endif /* ndef __INTERFLOP_MCAQUAD_H */
