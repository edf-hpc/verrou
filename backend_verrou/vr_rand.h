/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for random number generation.                      ---*/
/*---                                                    vr_rand.h ---*/
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

#ifndef __VR_RAND_H
#define __VR_RAND_H

//#include "pub_tool_basics.h"

typedef struct Vr_Rand_ Vr_Rand;
struct Vr_Rand_ {
  int current_;
  int count_;
  unsigned long int next_;
  unsigned int seed_;
  int reload_;
};

//extern Vr_Rand vr_rand;

Vr_Rand vr_rand;

#include "vr_rand_implem.h"



/* void vr_rand_setSeed (Vr_Rand * r, unsigned int c); */
/* unsigned int vr_rand_getSeed (Vr_Rand * r); */
/* bool vr_rand_bool (Vr_Rand * r); */
/* int vr_rand_int (Vr_Rand * r); */
/* int vr_rand_max (void); */



#endif
