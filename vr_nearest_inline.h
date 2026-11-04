
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- NEAREST operators inlined in the generated wrappers.         ---*/
/*---                                            vr_nearest_inline.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
     F. Févotte <francois.fevotte@edf.fr>
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

// Same results as interflop_verrou_*_NEAREST, without the call to the backend. vr_main.c has to be
// compiled with -ffp-contract=off : the mul and the add of an unfused FMA must not be merged.

#define VR_NEAREST_BINARY(NAME, OP, TYPE)                               \
   static inline void vr_nearest_##NAME##_##TYPE (TYPE a, TYPE b, TYPE* res, void* context) { \
      *res=a OP b;                                                      \
   }

VR_NEAREST_BINARY(add, +, double)
VR_NEAREST_BINARY(sub, -, double)
VR_NEAREST_BINARY(mul, *, double)
VR_NEAREST_BINARY(div, /, double)
VR_NEAREST_BINARY(add, +, float)
VR_NEAREST_BINARY(sub, -, float)
VR_NEAREST_BINARY(mul, *, float)
VR_NEAREST_BINARY(div, /, float)

static inline void vr_nearest_cast_double_to_float (double a, float* res, void* context) {
   *res=(float)a;
}

#ifdef USE_VERROU_SQRT
// __builtin_sqrt may call sqrt, which is not available in the tool
static inline void vr_nearest_sqrt_double (double a, double* res, void* context) {
   interflop_verrou_sqrt_double_NEAREST(a, res, context);
}

static inline void vr_nearest_sqrt_float (float a, float* res, void* context) {
   interflop_verrou_sqrt_float_NEAREST(a, res, context);
}
#endif

#ifdef USE_VERROU_FMA
static inline void vr_nearest_madd_double (double a, double b, double c, double* res, void* context) {
   *res=__builtin_fma(a, b, c);
}

static inline void vr_nearest_madd_float (float a, float b, float c, float* res, void* context) {
   *res=__builtin_fmaf(a, b, c);
}
#endif
