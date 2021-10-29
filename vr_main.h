
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
  VR_PREC_DBL_TO_FLT,
  VR_PREC_FLT_TO_DBL,
  VR_PREC_DBL_TO_INT,
  VR_PREC_DBL_TO_SHT,
  VR_PREC_FLT_TO_INT,
  VR_PREC_FLT_TO_SHT,
  VR_PREC
} Vr_Prec;


typedef struct Vr_Exclude_ Vr_Exclude;
struct Vr_Exclude_ {
  HChar*      fnname;
  HChar*      objname;
  Bool        used;
  Vr_Exclude* next;
};

typedef struct Vr_Include_Trace_ Vr_Include_Trace;
struct Vr_Include_Trace_ {
  HChar*      fnname;
  HChar*      objname;
  Vr_Include_Trace* next;
};


typedef struct Vr_IncludeSource_ Vr_IncludeSource;
struct Vr_IncludeSource_ {
  HChar*            fnname;
  HChar*            filename;
  UInt              linenum;
  Vr_IncludeSource* next;
};

typedef struct {
  vr_backend_name_t backend;
  enum vr_RoundingMode roundingMode;
  Bool count;
  Bool instr_op[VR_OP];
  Bool instr_vec[VR_VEC];
  Bool instr_prec[VR_PREC];

  Vr_Instr instrument;
  Bool verbose;
  Bool unsafe_llo_optim;

  UInt firstSeed;

  Bool genExclude;
  HChar * excludeFile;
  //  HChar * genAbove;
  Vr_Exclude * exclude;
  Vr_Exclude * genExcludeUntil;

  Bool genIncludeSource;
  HChar* includeSourceFile;

  Bool sourceActivated;
  Vr_IncludeSource *includeSource;
  Vr_IncludeSource *genIncludeSourceUntil;

  Bool sourceExcludeActivated;
  Vr_IncludeSource *excludeSourceRead;
  Vr_IncludeSource *excludeSourceDyn;

  UInt mca_precision_double;
  UInt mca_precision_float;
  UInt mca_mode;

  Bool checknan;

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
  HChar* outputTraceRep;
} Vr_State;

extern Vr_State vr;


// * Functions declarations

// ** vr_main.c
UInt vr_count_fp_instrumented (void);
UInt vr_count_fp_not_instrumented (void);
void vr_ppOpCount (void);
void vr_cancellation_handler(int cancelled );
void vr_denorm_handler(void);
void vr_float_max_handler(void);



// ** vr_clreq.c

Bool vr_handle_client_request (ThreadId tid, UWord *args, UWord *ret);
void vr_set_instrument_state (const HChar* reason, Vr_Instr state, Bool discard);


// ** vr_error.c

typedef enum {
  VR_ERROR_UNCOUNTED,
  VR_ERROR_SCALAR,
  VR_ERROR_NAN,
  VR_ERROR_CC,
  VR_ERROR_CD,
  VR_ERROR_FLT_MAX,
  VR_ERROR
} Vr_ErrorKind;

const HChar* vr_get_error_name (const Error* err);
Bool vr_recognised_suppression (const HChar* name, Supp* su);
void vr_before_pp_Error (const Error* err) ;
void vr_pp_Error (const Error* err);
Bool vr_eq_Error (VgRes res, const Error* e1, const Error* e2);
UInt vr_update_extra (const Error* err);
Bool vr_error_matches_suppression (const Error* err, const Supp* su);
Bool vr_read_extra_suppression_info (Int fd, HChar** bufpp, SizeT* nBuf,
                                     Int* lineno, Supp* su);
SizeT vr_print_extra_suppression_info (const Error* er,
                                      /*OUT*/HChar* buf, Int nBuf);
SizeT vr_print_extra_suppression_use (const Supp* s,
                                     /*OUT*/HChar* buf, Int nBuf);
void vr_update_extra_suppression_use (const Error* err, const Supp* su);


void vr_maybe_record_ErrorOp (Vr_ErrorKind kind, IROp op);
void vr_maybe_record_ErrorRt (Vr_ErrorKind kind);
void vr_handle_NaN (void);
void vr_handle_CC (int);
void vr_handle_CD (void);
void vr_handle_FLT_MAX (void);


// ** vr_exclude.c

void        vr_freeExcludeList (Vr_Exclude* list);
void        vr_dumpExcludeList (Vr_Exclude* list, Vr_Exclude* end,
                                const HChar* filename);
Vr_Exclude* vr_loadExcludeList (Vr_Exclude * list, const HChar * filename);
Bool        vr_excludeIRSB(const HChar** fnname, const HChar** objname);
void        vr_excludeIRSB_generate(const HChar** fnname, const HChar** objname);

void vr_freeIncludeSourceList (Vr_IncludeSource* list);
void vr_dumpIncludeSourceList (Vr_IncludeSource* list, Vr_IncludeSource* end,
                               const HChar* fname);
Vr_IncludeSource * vr_loadIncludeSourceList (Vr_IncludeSource * list, const HChar * fname);
Bool vr_includeSource (Vr_IncludeSource** list,
                       const HChar* fnname, const HChar* filename, UInt linenum);
void vr_includeSource_generate (Vr_IncludeSource** list,
				const HChar* fnname, const HChar* filename, UInt linenum);

Vr_IncludeSource * vr_addIncludeSource (Vr_IncludeSource* list, const HChar* fnname,
					const HChar * filename, UInt linenum);

// ** vr_include_trace.c
void vr_freeIncludeTraceList (Vr_Include_Trace* list) ;
Vr_Include_Trace * vr_loadIncludeTraceList (Vr_Include_Trace * list, const HChar * fname);
Bool vr_includeTraceIRSB (const HChar** fnname, const HChar **objname);


//**  vr_traceBB.c


void vr_traceBB_resetCov(void);
UInt vr_traceBB_dumpCov(void);

#define VR_FNNAME_BUFSIZE 4096


// ** vr_clo.c

void vr_env_clo (const HChar* env, const HChar *clo);
void vr_clo_defaults (void);
Bool vr_process_clo (const HChar *arg);
void vr_print_usage (void);
void vr_print_debug_usage (void);


#endif /*ndef __VR_MAIN_H*/
