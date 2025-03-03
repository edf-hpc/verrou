
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floatin-point operations overloading           ---*/
/*--- designed to detect denormal and apply flush to zero          ---*/
/*---                               interflop_checkdenorm.h        ---*/
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

#ifndef __INTERFLOP_CHECKDENORM_H
#define __INTERFLOP_CHECKDENORM_H

//#define DEBUG_PRINT_OP

#ifdef __cplusplus
extern "C" {
#endif
#define IFCD_FCTNAME(FCT) interflop_checkdenorm_##FCT

#include "../interflop.h"

  struct checkdenorm_conf {
      int flushtozero; //bool
      int denormarezero;//bool
      int counter;
  };

  typedef struct checkdenorm_conf checkdenorm_conf_t;

  typedef enum check_subnormal_op {
    CDN_ADD=0,
    CDN_SUB,
    CDN_MUL,
    CDN_DIV,
    CDN_MADD,
    CDN_SQRT,
    CDN_CAST,
    CDN_OP_SIZE //to count the number of element (always last element of enum)
  } check_subnormal_op_t;

  typedef enum check_subnormal_type {
    CDN_FLOAT=0,
    CDN_DOUBLE,
    CDN_TYPE_SIZE //to count the number of element (always last element of enum)
  } check_subnormal_type_t;


  const char* check_denorm_op_name (check_subnormal_op_t op);
  const char* check_denorm_type_name (check_subnormal_type_t type);
  unsigned int  check_denorm_nb_args (check_subnormal_op_t op);

  void IFCD_FCTNAME(configure)(checkdenorm_conf_t mode,void* context);
  void IFCD_FCTNAME(finalize)(void* context);

  const char* IFCD_FCTNAME(get_backend_name)(void);
  const char* IFCD_FCTNAME(get_backend_version)(void);

  void checkdenorm_set_denorm_output_handler(void (*)(check_subnormal_op_t, check_subnormal_type_t));
  void checkdenorm_set_denorm_input_handler(void (*)(check_subnormal_op_t, check_subnormal_type_t, unsigned int));

  void checkdenorm_reset_counter(void);
  typedef unsigned int (*myPrintfType)(const char *format, ...);
  void checkdenorm_print_counter(myPrintfType myPrintf);


  extern void (*vr_panicHandler)(const char*);
  void checkdenorm_set_panic_handler(void (*)(const char*));

  struct interflop_backend_interface_t IFCD_FCTNAME(init)(void ** context);

  void IFCD_FCTNAME(add_double) (double a, double b, double* res, void* context);    
  void IFCD_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFCD_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFCD_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFCD_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFCD_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFCD_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFCD_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFCD_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFCD_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFCD_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFCD_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFCD_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);

#ifdef __cplusplus
}
#endif

#endif /* ndef __INTERFLOP_CHECKDENORM_H */
