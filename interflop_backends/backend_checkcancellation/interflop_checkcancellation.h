
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floatin-point operations overloading           ---*/
/*--- designed to detect cancellation                              ---*/
/*---                               interflop_checkcancellation_.h ---*/
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

#ifndef __INTERFLOP_CHECKCANCELLATION_H
#define __INTERFLOP_CHECKCANCELLATION_H

//#define DEBUG_PRINT_OP

#ifdef __cplusplus
extern "C" {
#endif
#define IFCC_FCTNAME(FCT) interflop_checkcancellation_##FCT

#include "../interflop.h"

   struct checkcancellation_conf {
     unsigned int threshold_float;
     unsigned int threshold_double;
  };

  typedef struct checkcancellation_conf checkcancellation_conf_t;


  void IFCC_FCTNAME(configure)(checkcancellation_conf_t mode,void* context);
  void IFCC_FCTNAME(finalize)(void* context);

  const char* IFCC_FCTNAME(get_backend_name)(void);
  const char* IFCC_FCTNAME(get_backend_version)(void);

  void checkcancellation_set_cancellation_handler(void (*)(int));

  extern void (*vr_panicHandler)(const char*);
  void checkcancellation_set_panic_handler(void (*)(const char*));

  struct interflop_backend_interface_t IFCC_FCTNAME(init)(void ** context);
   
  void IFCC_FCTNAME(add_double) (double a, double b, double* res, void* context);    
  void IFCC_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFCC_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFCC_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
   //void IFCC_FCTNAME(mul_double) (double a, double b, double* res, void* context);
   //void IFCC_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
   //void IFCC_FCTNAME(div_double) (double a, double b, double* res, void* context);
   //void IFCC_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

   //void IFCC_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFCC_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFCC_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);

  
#ifdef __cplusplus
}
#endif

#endif /* ndef __INTERFLOP_CHECKCANCELLATION_H */
