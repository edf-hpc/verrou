
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Header file for core valgrind-based features.                ---*/
/*---                                                    vr_main.h ---*/
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
#include "vr_fpOps.h"


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
  VR_OP
} Vr_Op;

typedef struct Vr_Exclude_ Vr_Exclude;
struct Vr_Exclude_ {
  HChar*      fnname;
  HChar*      objname;
  Bool        used;
  Vr_Exclude* next;
};

typedef struct Vr_IncludeSource_ Vr_IncludeSource;
struct Vr_IncludeSource_ {
  HChar*            fnname;
  HChar*            filename;
  UInt              linenum;
  Vr_IncludeSource* next;
};

typedef struct {
  enum vr_RoundingMode roundingMode;
  Bool count;
  Bool instr_op[VR_OP];
  Bool instr_scalar;
  Vr_Instr instrument;
  Bool verbose;

  Bool genExclude;
  HChar * excludeFile;
  Vr_Exclude * exclude;
  Vr_Exclude * genExcludeUntil;

  Bool genIncludeSource;
  HChar* includeSourceFile;
  Vr_IncludeSource *includeSource;
  Vr_IncludeSource *genIncludeSourceUntil;
} Vr_State;

extern Vr_State vr;


// * Functions declarations

// ** vr_main.c

void vr_ppOpCount (void);


// ** vr_clreq.c

Bool vr_handle_client_request (ThreadId tid, UWord *args, UWord *ret);


// ** vr_error.c

typedef enum {
  VR_ERROR_UNCOUNTED,
  VR_ERROR_SCALAR,
  VR_ERROR
} Vr_ErrorKind;

const HChar* vr_get_error_name (Error* err);
Bool vr_recognised_suppression (const HChar* name, Supp* su);
void vr_before_pp_Error (Error* err) ;
void vr_pp_Error (Error* err);
Bool vr_eq_Error (VgRes res, Error* e1, Error* e2);
UInt vr_update_extra (Error* err);
Bool vr_error_matches_suppression (Error* err, Supp* su);
Bool vr_read_extra_suppression_info (Int fd, HChar** bufpp, SizeT* nBuf,
                                     Int* lineno, Supp* su);
Bool vr_print_extra_suppression_info (Error* er,
                                      /*OUT*/HChar* buf, Int nBuf);
Bool vr_print_extra_suppression_use (Supp* s,
                                     /*OUT*/HChar* buf, Int nBuf);
void vr_update_extra_suppression_use (Error* err, Supp* su);


void vr_maybe_record_ErrorOp (Vr_ErrorKind kind, IROp op);


// ** vr_exclude.c

void        vr_freeExcludeList (Vr_Exclude* list);
void        vr_dumpExcludeList (Vr_Exclude* list, Vr_Exclude* end,
                                const HChar* filename);
Vr_Exclude* vr_loadExcludeList (Vr_Exclude * list, const HChar * filename);
Bool        vr_excludeIRSB(Vr_Exclude** list, Bool generate,
                           HChar* fnname, HChar* objname);

void vr_freeIncludeSourceList (Vr_IncludeSource* list);
void vr_dumpIncludeSourceList (Vr_IncludeSource* list, Vr_IncludeSource* end,
                               const HChar* fname);
Vr_IncludeSource * vr_loadIncludeSourceList (Vr_IncludeSource * list, const HChar * fname);
Bool vr_includeSource (Vr_IncludeSource** list, Bool generate,
                       HChar* fnname, HChar* filename, UInt linenum);


// ** vr_clo.c

void vr_env_clo (const HChar* env, const HChar *clo);
void vr_clo_defaults (void);
Bool vr_process_clo (const HChar *arg);
void vr_print_usage (void);
void vr_print_debug_usage (void);


#define VR_FNNAME_BUFSIZE 4096

#endif /*ndef __VR_MAIN_H*/
