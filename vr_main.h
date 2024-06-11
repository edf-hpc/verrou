
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Header file for core valgrind-based features.                ---*/
/*---                                                    vr_main.h ---*/
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

#ifndef __VR_MAIN_H
#define __VR_MAIN_H

#include "pub_tool_basics.h"
#include "pub_tool_vki.h"
#include "pub_tool_debuginfo.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcproc.h"
#include "pub_tool_machine.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_options.h"
#include "pub_tool_oset.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_xarray.h"
#include "pub_tool_clientstate.h"
#include "pub_tool_machine.h"
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"
#include "pub_tool_gdbserver.h"

#include "verrou.h"

#include "interflop_backends/statically_integrated_backends.h"

#include "vr_exclude.h"
#include "vr_error.h"
#include "vr_include_trace.h"

#define VR_FNNAME_BUFSIZE 4096

typedef enum vr_backend_name{vr_verrou,vr_mcaquad, vr_checkdenorm} vr_backend_name_t;

typedef enum vr_backendpost_name{vr_nopost,vr_checkcancellation, vr_check_float_max} vr_backendpost_name_t;


// * Type declarations

typedef enum {
  VR_INSTR_OFF,
  VR_INSTR_ON,
  VR_INSTR
} Vr_Instr;

typedef enum {
  VR_OP_ADD,    // Addition
  VR_OP_SUB,    // Subtraction
  VR_OP_MUL,    // Multiplication
  VR_OP_DIV,    // Division
  VR_OP_MADD,   // FMA ADD
  VR_OP_MSUB,   // FMA SUB
  VR_OP_SQRT,   // Sqrt
  VR_OP_CMP,    // Comparison
  VR_OP_CONV,    // Conversion
  VR_OP_MAX,    // Maximum
  VR_OP_MIN,    // Minimum
  VR_OP
} Vr_Op; //Warning : Operation after   VR_OP_CMP are not instrumented

// *** Vector operations
typedef enum {
  VR_VEC_SCAL,  // Scalar operation
  VR_VEC_LLO,   // Vector operation, lowest lane only
  VR_VEC_FULL2,  // Vector operation
  VR_VEC_FULL4,  // Vector operation
  VR_VEC_FULL8,  // Vector operation
  VR_VEC_UNK,
  VR_VEC
} Vr_Vec;

// *** Operation precision
typedef enum {
  VR_PREC_FLT,  // Single
  VR_PREC_DBL,  // Double
  VR_PREC_LDBL,  // LDouble
  VR_PREC_DBL_TO_FLT,
  VR_PREC_FLT_TO_DBL,
  VR_PREC_DBL_TO_INT,
  VR_PREC_DBL_TO_SHT,
  VR_PREC_FLT_TO_INT,
  VR_PREC_FLT_TO_SHT,
  VR_PREC
} Vr_Prec;

typedef enum {
  VR_PRANDOM_UPDATE_NONE,
  VR_PRANDOM_UPDATE_FUNC,
} Vr_Prandom_update;




typedef struct {
  vr_backend_name_t backend;
  enum vr_RoundingMode roundingMode;
  enum vr_RoundingMode roundingModeNoInst;
  Vr_Prandom_update prandomUpdate;
  double prandomFixedInitialValue;
  Bool count;
  Bool instr_op[VR_OP];
  Bool instr_vec[VR_VEC];
  Bool instr_prec[VR_PREC];

//  Vr_Instr instrument;
  Vr_Instr instrument_hard;
  Vr_Instr instrument_soft;
  Bool instrument_soft_used;

  Bool verbose;
  Bool unsafe_llo_optim;

  ULong firstSeed;

  Bool genExcludeBool;
  HChar * excludeFile;
  //  HChar * genAbove;
  Vr_Exclude * exclude;
  Vr_Exclude * gen_exclude;
  Bool excludeDetect;
  Bool loadInterLibm;

  Bool genIncludeSource;
  HChar* includeSourceFile;

  Bool sourceActivated;
  Vr_IncludeSource *includeSource;
  Vr_IncludeSource *gen_includeSource;

  Bool sourceExcludeActivated;
  Vr_IncludeSource *excludeSourceRead;

  UInt mca_precision_double;
  UInt mca_precision_float;
  UInt mca_mode;

  Bool checknan;
  Bool checkinf;

  Bool checkCancellation;
  UInt cc_threshold_double;
  UInt cc_threshold_float;

  Bool dumpCancellation;
  HChar* cancellationDumpFile;
  Vr_IncludeSource * cancellationSource;

  Bool checkDenorm;
  Bool ftz;
  Bool dumpDenorm ;
  HChar* denormDumpFile;
  Vr_IncludeSource * denormSource;

  Bool checkFloatMax;

  Bool genTrace;
  Vr_Include_Trace* includeTrace;

  Bool useIOMatchCLR;
  HChar* IOMatchScript;
  Int IOMatchCLRFileInput;

  HChar* outputIOMatchFilePattern;
  Int IOMatchFileDescriptor;

  HChar* outputIOMatchRep;
  HChar* outputTraceRep;

} Vr_State;

extern Vr_State vr;

#include "vr_clreq.h"

// Functions declarations
UInt vr_count_fp_instrumented (void);
UInt vr_count_fp_not_instrumented (void);
void vr_ppOpCount (void);
void vr_resetCount(void);

void vr_register_cache(unsigned int*, unsigned int);
void vr_clean_cache(void);
void vr_register_cache_seed(unsigned int*);
void vr_clean_cache_seed(void);


/* Implem in vr_traceBB_impl.h*/
void vr_traceBB_resetCov(void);
UInt vr_traceBB_dumpCov(void);

#include "vr_clo.h"
#include "vr_expect_clr.h"


#endif /*ndef __VR_MAIN_H*/
