
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
   published by the Free Software Foundation; either version 3 of the
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
#include "pub_tool_clreq.h"
#endif

/* Version of the client request API defined by this file. It is incremented
   when a request is added (the existing requests are frozen, see below).
   VERROU_GET_API_VERSION returns the version of the running verrou: 0 when
   the program does not run under verrou, or runs under a verrou older than
   the request (version 1). */
#define VERROU_API_VERSION 1

/* The macros only use standard types: in a client program, this file needs
   no definition from the user (Bool is an unsigned char, a rounding mode an
   int: the value of enum vr_RoundingMode of interflop_verrou.h, which is the
   type of the rounding mode when interflop_verrou.h is included before). */
#ifndef __PUB_TOOL_BASICS_H
#include <stdint.h>
#endif
#ifdef __INTERFLOP_VERROU_H
#define VERROU_ROUNDING_MODE_TYPE enum vr_RoundingMode
#else
#define VERROU_ROUNDING_MODE_TYPE int
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
  VR_USERREQ__GET_SEED,
  VR_USERREQ__REGISTER_CACHE_SEED,
  VR_USERREQ__GET_LIBM_ROUNDING_NO_INST,
  VR_USERREQ__START_SOFT_INSTRUMENTATION,
  VR_USERREQ__STOP_SOFT_INSTRUMENTATION,
  VR_USERREQ__FLOAT_CONV,
  VR_USERREQ__PRINT_COUNT_DENORM,
  VR_USERREQ__RESET_COUNT_DENORM,
  VR_USERREQ__SET_ROUNDING_MODE,
  VR_USERREQ__GET_API_VERSION
} Vg_VerrouClientRequest;

#define VERROU_START_INSTRUMENTATION                                 \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_INSTRUMENTATION, \
                                  0, 0, 0, 0, 0)

#define VERROU_STOP_INSTRUMENTATION                                  \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_INSTRUMENTATION,  \
                                  0, 0, 0, 0, 0)
#define VERROU_START_SOFT_INSTRUMENTATION                                 \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_SOFT_INSTRUMENTATION, \
                                  0, 0, 0, 0, 0)

#define VERROU_STOP_SOFT_INSTRUMENTATION                                  \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_SOFT_INSTRUMENTATION,  \
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

#define VERROU_SET_ROUNDING_MODE(ROUNDINGMODE)                             \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__SET_ROUNDING_MODE,    \
                                  ROUNDINGMODE, 0, 0, 0, 0)


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
  (VERROU_ROUNDING_MODE_TYPE)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                                VR_USERREQ__GET_ROUNDING, \
					    0, 0, 0, 0, 0)
#define VERROU_GET_LIBM_ROUNDING \
  (VERROU_ROUNDING_MODE_TYPE)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                                VR_USERREQ__GET_LIBM_ROUNDING, \
					    0, 0, 0, 0, 0)
#define VERROU_GET_LIBM_ROUNDING_NO_INST \
  (VERROU_ROUNDING_MODE_TYPE)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                                VR_USERREQ__GET_LIBM_ROUNDING_NO_INST, \
					    0, 0, 0, 0, 0)
#define VERROU_NAN_DETECTED \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__NAN_DETECTED,    \
                                  0, 0, 0, 0, 0)
#define VERROU_INF_DETECTED \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__INF_DETECTED,    \
                                  0, 0, 0, 0, 0)
#define VERROU_IS_INSTRUMENTED_FLOAT \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_FLOAT,\
					    0, 0, 0, 0, 0)

#define VERROU_IS_INSTRUMENTED_DOUBLE \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_DOUBLE,\
					    0, 0, 0, 0, 0)
#define VERROU_IS_INSTRUMENTED_LDOUBLE \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_LDOUBLE,\
					    0, 0, 0, 0, 0)
#define VERROU_COUNT_OP \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__COUNT_OP,\
					    0, 0, 0, 0, 0)

#define VERROU_GENERATE_EXCLUDE_SOURCE(FCTNAME, LINENUM, FILENAME, OBJECT) \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__GENERATE_EXCLUDE_SOURCE,    \
				  FCTNAME, LINENUM, FILENAME, OBJECT, 0)


#define VERROU_IS_INSTRUMENTED_EXCLUDE_SOURCE(FCTNAME, LINENUM, FILENAME, OBJECT) \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
					    VR_USERREQ__IS_INSTRUMENTED_EXCLUDE_SOURCE,\
					FCTNAME, LINENUM, FILENAME, OBJECT, 0)

#define VERROU_REGISTER_CACHE(CACHEPTR, SIZE)                 \
   VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__REGISTER_CACHE,\
                                   CACHEPTR,SIZE, 0, 0, 0)

#define VERROU_GET_SEED \
  (uint64_t) VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	  \
					     VR_USERREQ__GET_SEED,\
					     0, 0, 0, 0, 0)

#define VERROU_REGISTER_CACHE_SEED(CACHEPTR)			\
   VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__REGISTER_CACHE_SEED,\
                                   CACHEPTR,0, 0, 0, 0)

#define VERROU_FLOAT_CONV \
  (unsigned char)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	              \
                                        VR_USERREQ__FLOAT_CONV,       \
                                        0, 0, 0, 0, 0)

#define VERROU_PRINT_DENORM_COUNTER \
  VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	         \
                                  VR_USERREQ__PRINT_COUNT_DENORM,\
                                  0, 0, 0, 0, 0)
#define VERROU_RESET_DENORM_COUNTER \
  VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	         \
                                  VR_USERREQ__RESET_COUNT_DENORM,\
                                  0, 0, 0, 0, 0)

#define VERROU_GET_API_VERSION \
  (unsigned int)VALGRIND_DO_CLIENT_REQUEST_EXPR(0 /* if not */,	      \
                                                VR_USERREQ__GET_API_VERSION, \
                                                0, 0, 0, 0, 0)
#endif /* __VERROU_H */
