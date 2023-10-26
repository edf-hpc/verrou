
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Headers for the public API.                                  ---*/
/*---                                                     verrou.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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

#ifndef __VERROU_H
#define __VERROU_H

#ifdef VERROU_SYNCHRO_INCLUDE
#include "../include/valgrind.h"
#else
#include "valgrind.h"
#endif
/* !! ABIWARNING !! ABIWARNING !! ABIWARNING !! ABIWARNING !!
   This enum comprises an ABI exported by Valgrind to programs
   which use client requests.  DO NOT CHANGE THE ORDER OF THESE
   ENTRIES, NOR DELETE ANY -- add new ones at the end.
 */

typedef
enum {
  VR_USERREQ__START_INSTRUMENTATION = VG_USERREQ_TOOL_BASE('V', 'R'),
  VR_USERREQ__STOP_INSTRUMENTATION,
  VR_USERREQ__START_DETERMINISTIC,
  VR_USERREQ__STOP_DETERMINISTIC,
  VR_USERREQ__DISPLAY_COUNTERS,
  VR_USERREQ__DUMP_COVER,
  VR_USERREQ__COUNT_FP_INSTRUMENTED,
  VR_USERREQ__COUNT_FP_NOT_INSTRUMENTED,
  VR_USERREQ__PRINT_PROFILING_EXACT,
  VR_USERREQ__SET_SEED,
  VR_USERREQ__PRANDOM_UPDATE,
  VR_USERREQ__PRANDOM_UPDATE_VALUE,
  VR_USERREQ__GET_ROUNDING,
  VR_USERREQ__GET_LIBM_ROUNDING,
  VR_USERREQ__NAN_DETECTED,
  VR_USERREQ__INF_DETECTED,
  VR_USERREQ__IS_INSTRUMENTED_FLOAT,
  VR_USERREQ__IS_INSTRUMENTED_DOUBLE,
  VR_USERREQ__IS_INSTRUMENTED_LDOUBLE,
  VR_USERREQ__COUNT_OP,
  VR_USERREQ__GENERATE_EXCLUDE_SOURCE,
  VR_USERREQ__IS_INSTRUMENTED_EXCLUDE_SOURCE,
  VR_USERREQ__REGISTER_CACHE,
} Vg_VerrouClientRequest;

#define VERROU_START_INSTRUMENTATION                                 \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_INSTRUMENTATION, \
                                  0, 0, 0, 0, 0)

#define VERROU_STOP_INSTRUMENTATION                                  \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_INSTRUMENTATION,  \
                                  0, 0, 0, 0, 0)

#define VERROU_START_DETERMINISTIC(LEVEL)                            \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_DETERMINISTIC,   \
                                  LEVEL, 0, 0, 0, 0)

#define VERROU_STOP_DETERMINISTIC(LEVEL)                             \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_DETERMINISTIC,    \
                                  LEVEL, 0, 0, 0, 0)
#define VERROU_SET_SEED(SEED)                             \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__SET_SEED,    \
                                  SEED, 0, 0, 0, 0)

#define VERROU_DISPLAY_COUNTERS                                      \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__DISPLAY_COUNTERS,      \
                                  0, 0, 0, 0, 0)
#define VERROU_PRINT_PROFILING_EXACT                                      \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__PRINT_PROFILING_EXACT,	\
                                  0, 0, 0, 0, 0)

#define VERROU_DUMP_COVER \
  (unsigned int)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	      \
					    VR_USERREQ__DUMP_COVER,   \
					    0, 0, 0, 0, 0)

#define VERROU_COUNT_FP_INSTRUMENTED \
  (unsigned int)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__COUNT_FP_INSTRUMENTED,\
					    0, 0, 0, 0, 0)

#define VERROU_COUNT_FP_NOT_INSTRUMENTED \
  (unsigned int)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__COUNT_FP_NOT_INSTRUMENTED,\
					    0, 0, 0, 0, 0)


#define VERROU_PRANDOM_UPDATE \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__PRANDOM_UPDATE,    \
                                  0, 0, 0, 0, 0)
#define VERROU_PRANDOM_UPDATE_VALUE(P)\
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__PRANDOM_UPDATE_VALUE,    \
				  P, 0, 0, 0, 0)

#define VERROU_GET_ROUNDING \
  (vr_RoundingMode)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                                VR_USERREQ__GET_ROUNDING, \
					    0, 0, 0, 0, 0)
#define VERROU_GET_LIBM_ROUNDING \
  (vr_RoundingMode)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                                VR_USERREQ__GET_LIBM_ROUNDING, \
					    0, 0, 0, 0, 0)

#define VERROU_NAN_DETECTED \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__NAN_DETECTED,    \
                                  0, 0, 0, 0, 0)
#define VERROU_INF_DETECTED \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__INF_DETECTED,    \
                                  0, 0, 0, 0, 0)
#define VERROU_IS_INSTRUMENTED_FLOAT \
  (Bool)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_FLOAT,\
					    0, 0, 0, 0, 0)

#define VERROU_IS_INSTRUMENTED_DOUBLE \
  (Bool)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_DOUBLE,\
					    0, 0, 0, 0, 0)
#define VERROU_IS_INSTRUMENTED_LDOUBLE \
  (Bool)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_LDOUBLE,\
					    0, 0, 0, 0, 0)
#define VERROU_COUNT_OP \
  (Bool)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__COUNT_OP,\
					    0, 0, 0, 0, 0)

#define VERROU_GENERATE_EXCLUDE_SOURCE(FCTNAME, LINENUM, FILENAME, OBJECT) \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__GENERATE_EXCLUDE_SOURCE,    \
				  FCTNAME, LINENUM, FILENAME, OBJECT, 0)


#define VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(FCTNAME, LINENUM, FILENAME, OBJECT) \
  (Bool)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_EXCLUDE_SOURCE,\
					FCTNAME, LINENUM, FILENAME, OBJECT, 0)

#define VERROU_REGISTER_CACHE(CACHEPTR, SIZE)                 \
   VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__REGISTER_CACHE,\
                                   CACHEPTR,SIZE, 0, 0, 0)

#endif /* __VERROU_H */
