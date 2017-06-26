
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

#ifndef __VR_FPOPS_H
#define __VR_FPOPS_H


#ifdef __cplusplus
extern "C" {
#endif
#define BACKENDNAME
#include "interflop_backend_interface.h"


  
  enum vr_RoundingMode {
    VR_NEAREST,
    VR_UPWARD,
    VR_DOWNWARD,
    VR_ZERO,
    VR_RANDOM, // Must be immediately after standard rounding modes
    VR_AVERAGE,
    VR_FARTHEST
  };

  //  int CHECK_C  = 0;

  
  void vr_fpOpsInit (enum vr_RoundingMode mode);
  void vr_fpOpsFini (void);

  void vr_beginInstrumentation(void);
  void vr_endInstrumentation(void);

  void vr_fpOpsSeed (unsigned int seed);
  void vr_fpOpsRandom (void);

  void setCancellationHandler(void (*cancellationHandler)(int));
  
#ifdef __cplusplus
}
#endif

#endif /* ndef __VR_FPOPS_H */
