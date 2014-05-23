
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                vr_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014
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

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"


// From cachegrind/cg_main.c
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
#include "pub_tool_machine.h"      // VG_(fnptr_to_fnentry)

#include "verrou.h"
#include "vr_fpOps.h"
#include <fenv.h>
//#pragma STDC FENV_ACCESS ON

/* * Global features
 */

/* ** Start-stop instrumentation
 */
Bool vr_instrument_state = True;

static void vr_set_instrument_state (const HChar* reason, Bool state) {
  if (vr_instrument_state == state) {
    VG_(message)(Vg_DebugMsg, "%s: instrumentation already %s\n",
		 reason, state ? "ON" : "OFF");
    return;
  }

  vr_instrument_state = state;
  VG_(message)(Vg_DebugMsg, "%s: instrumentation switched %s\n",
               reason, state ? "ON" : "OFF");
}

/* ** Enter/leave deterministic section
 */
HChar vr_deterministic_section[256];

static VG_REGPARM(1) void vr_set_deterministic_section (const ULong addr) {
  HChar fnname[256];
  HChar filename[256];
  UInt  linenum;

  fnname[0] = 0;
  VG_(get_fnname)(addr, fnname, 256);

  filename[0] = 0;
  VG_(get_filename_linenum)(addr,
                            filename, 256,
                            NULL,     0,
                            NULL,
                            &linenum);
  VG_(snprintf)(vr_deterministic_section, 256,
                "%s (%s:%d)", fnname, filename, linenum);
}

static void vr_start_deterministic_section (void) {
  unsigned int hash = VG_(getpid)();
  {
    int i = 0;
    while (vr_deterministic_section[i] != 0) {
      hash += i * vr_deterministic_section[i];
      ++i;
    }
  }

  vr_fpOpsSeed (hash);
  VG_(message)(Vg_DebugMsg, "Entering deterministic section %d: %s\n",
               hash, vr_deterministic_section);
}

static void vr_stop_deterministic_section (void) {
  VG_(message)(Vg_DebugMsg, "Leaving deterministic section: %s\n",
               vr_deterministic_section);
  vr_fpOpsRandom ();
}


/* ** Command-line options
 */
typedef struct _vr_CLO vr_CLO;
struct _vr_CLO {
  enum vr_RoundingMode roundingMode;
};
vr_CLO vr_clo;

static Bool vr_process_clo (const HChar *arg) {
  Bool bool_val;

  if      (VG_XACT_CLO (arg, "--rounding-mode=random",
                        vr_clo.roundingMode, VR_RANDOM)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=average",
                        vr_clo.roundingMode, VR_AVERAGE)) {}

  else if (VG_BOOL_CLO (arg, "--instr-atstart",
                        bool_val)) {
    vr_set_instrument_state ("Command Line", bool_val);
  }
  return True;
}

static void vr_clo_defaults (void) {
  vr_clo.roundingMode = VR_NEAREST;
}

static void vr_print_usage (void) {

}

static void vr_print_debug_usage (void) {

}

/* ** Client requests
 */

static Bool vr_handle_client_request (ThreadId tid, UWord *args, UWord *ret) {
  if (!VG_IS_TOOL_USERREQ('V','R', args[0]))
    return False;

  switch (args[0]) {
  case VR_USERREQ__START_INSTRUMENTATION:
    vr_set_instrument_state ("Client Request", True);
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__STOP_INSTRUMENTATION:
    vr_set_instrument_state ("Client Request", False);
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__START_DETERMINISTIC:
    vr_start_deterministic_section ();
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__STOP_DETERMINISTIC:
    vr_stop_deterministic_section ();
    *ret = 0; /* meaningless */
    break;
  }
  return True;
}


/* * Floating-point operations counter
 */

/* ** Operation categories
 */

/* *** Operation type
 */
typedef enum {
  VR_OP_ADD,    // Addition
  VR_OP_SUB,    // Subtraction
  VR_OP_MUL,    // Multiplication
  VR_OP
} Vr_Op;

static const char* vr_ppOp (Vr_Op op) {
  switch (op) {
  case VR_OP_ADD:
    return "add";
  case VR_OP_SUB:
    return "sub";
  case VR_OP_MUL:
    return "mul";
  case VR_OP:
    break;
  }
  return "unknown";
}

/* *** Operation precision
 */
typedef enum {
  VR_PREC_FLT,  // Single
  VR_PREC_DBL,  // Double
  VR_PREC
} Vr_Prec;

static const char* vr_ppPrec (Vr_Prec prec) {
  switch (prec) {
  case VR_PREC_FLT:
    return "flt";
  case VR_PREC_DBL:
    return "dbl";
  case VR_PREC:
    break;
  }
  return "unknown";
}

/* *** Vector operations
 */
typedef enum {
  VR_VEC_SCAL,  // Scalar operation
  VR_VEC_LLO,   // Vector operation, lowest lane only
  VR_VEC_FULL,  // Vector operation
  VR_VEC
} Vr_Vec;

static const char* vr_ppVec (Vr_Vec vec) {
  switch (vec) {
  case VR_VEC_SCAL:
    return "scal";
  case VR_VEC_LLO:
    return "llo ";
  case VR_VEC_FULL:
    return "vec ";
  default:
    return "unknown";
  }
}

/* ** Counter handling
 */

static ULong vr_opCount[VR_OP][VR_PREC][VR_VEC];
static VG_REGPARM(2) void vr_incOpCount (ULong* counter, Long increment) {
  (*counter) += increment;
}

static void vr_countOp (IRSB* sb, Vr_Op op, Vr_Prec prec, Vr_Vec vec) {
  IRExpr** argv;
  IRDirty* di;
  int increment = 1;
  if (vec == VR_VEC_FULL) {
    switch (prec) {
    case VR_PREC_FLT:
      increment = 4;
      break;
    case VR_PREC_DBL:
      increment = 2;
      break;
    case VR_PREC:
      increment = 0;
    }
  }

  argv = mkIRExprVec_2 (mkIRExpr_HWord ((HWord)&vr_opCount[op][prec][vec]),
                        mkIRExpr_HWord (increment));

  di = unsafeIRDirty_0_N( 2,
                          "vr_incOpCount",
                          VG_(fnptr_to_fnentry)( &vr_incOpCount ),
                          argv);
  addStmtToIRSB (sb, IRStmt_Dirty (di));
}

static void vr_ppOpCount (void) {
  Vr_Op op;
  Vr_Prec prec;
  Vr_Vec vec;

  VG_(umsg)("    ------------------------------------\n");
  VG_(umsg)("    Operation\n");
  VG_(umsg)("     `- Precision\n");
  VG_(umsg)("         `- Vectorization          Count\n");
  VG_(umsg)("    ------------------------------------\n");
  for (op = 0 ; op<VR_OP ; ++op) {
    ULong countOp = 0;
    for (prec = 0 ; prec < VR_PREC ; ++prec) {
      for (vec = 0 ; vec < VR_VEC ; ++vec)
        countOp += vr_opCount[op][prec][vec];
    }

    if (countOp > 0) {
      VG_(umsg)("    %6s       %15llu\n",
                vr_ppOp(op), countOp);

      for (prec = 0 ; prec<VR_PREC ; ++prec) {
        ULong countPrec = 0;
        for (vec = 0 ; vec<VR_VEC ; ++vec)
          countPrec += vr_opCount[op][prec][vec];

        if (countPrec > 0) {
          VG_(umsg)("     `- %6s       %15llu\n",
                    vr_ppPrec(prec), countPrec);

          for (vec = 0 ; vec<VR_VEC ; ++vec) {
            if (vr_opCount[op][prec][vec] > 0) {
              VG_(umsg)("         `- %6s       %15llu\n",
                        vr_ppVec(vec),
                        vr_opCount[op][prec][vec]);
            }
          }
        }
      }
      VG_(umsg)("    ------------------------------------\n");
    }
  }
}


/* * Floating point operations overload
 */

/* ** Overloaded operators
 */

/* *** Addition
 */

static VG_REGPARM(2) Long vr_Add64F (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res = vr_AddDouble (*arg1, *arg2);
  Long *c = (Long*)(&res);
  return *c;
}

static VG_REGPARM(2) Long vr_Sub64F (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res = vr_AddDouble (*arg1, -(*arg2));
  Long *c = (Long*)(&res);
  return *c;
}

static VG_REGPARM(2) Int vr_Add32F (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res = vr_AddFloat (*arg1, *arg2);
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(2) Int vr_Sub32F (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res = vr_AddFloat (*arg1, -(*arg2));
  Int *c = (Int*)(&res);
  return *c;
}


/* ** Code instrumentation
 */

/* *** Helpers
 */

/* Return the Lowest Lane of a given packed temporary register
 */
static IRExpr* vr_getLL (IRSB* sb, IRType type, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, type);

  IROp op;
  switch (type) {
  case Ity_I32:
    op = Iop_V128to32;
    break;
  case Ity_I64:
    op = Iop_V128to64;
    break;
  default:
    op = Iop_INVALID;
  }
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (op, expr)));

  return IRExpr_RdTmp(tmp);
}

static IRExpr* vr_32to64 (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I64);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_32Uto64, expr)));
  return IRExpr_RdTmp (tmp);
}


/* Replace a given binary operation by a call to a function
 */
static void vr_replaceBinop (IRSB* sb, IRStmt* stmt, IRExpr* expr, IRType type,
                             const HChar* functionName, void* function,
                             Vr_Vec vec) {
  IRExpr * arg1 = expr->Iex.Binop.arg1;
  IRExpr * arg2 = expr->Iex.Binop.arg2;
  IRTemp res    = stmt->Ist.WrTmp.tmp;

  if (vec == VR_VEC_LLO) {
    arg1 = vr_getLL (sb, type, arg1);
    arg2 = vr_getLL (sb, type, arg2);
    res = newIRTemp (sb->tyenv, type);
  }

  if (type == Ity_I32) {
    arg1 = vr_32to64 (sb, arg1);
    arg2 = vr_32to64 (sb, arg2);
  }

  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1, arg2))));

  if (vec == VR_VEC_LLO) {
    IROp op;
    switch (type) {
    case Ity_I32:
      op = Iop_32UtoV128;
      break;
    case Ity_I64:
      op = Iop_64UtoV128;
      break;
    default:
      op = Iop_INVALID;
    }
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                     IRExpr_Unop (op, IRExpr_RdTmp(res))));
  }
}

static void vr_instrumentOp (IRSB* sb, IRStmt* stmt, IRExpr * expr, IROp op) {
    switch (op) {

      // Addition

      // - Double precision

    case Iop_AddF64: // Scalar
      vr_countOp (sb, VR_OP_ADD, VR_PREC_DBL, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Add64F0x2: // 128b vector, lowest-lane-only
      vr_countOp (sb, VR_OP_ADD, VR_PREC_DBL, VR_VEC_LLO);
      vr_replaceBinop (sb, stmt, expr, Ity_I64,
                       "vr_Add64F", vr_Add64F,
                       VR_VEC_LLO);
      break;

    case Iop_Add64Fx2: // 128b vector, 2 lanes
      vr_countOp (sb, VR_OP_ADD, VR_PREC_DBL, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;

      // - Single precision

    case Iop_AddF32: // Scalar
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Add32F0x4: // 128b vector, lowest-lane-only
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_LLO);
      vr_replaceBinop (sb, stmt, expr, Ity_I32,
                       "vr_Add32F", vr_Add32F,
                       VR_VEC_LLO);
      break;

    case Iop_Add32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;


      // Subtraction

      // - Double precision
    case Iop_SubF64: // Scalar
      vr_countOp (sb, VR_OP_SUB, VR_PREC_DBL, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Sub64F0x2: // 128b vector, lowest-lane only
      vr_countOp (sb, VR_OP_SUB, VR_PREC_DBL, VR_VEC_LLO);
      vr_replaceBinop (sb, stmt, expr, Ity_I64,
                       "vr_Sub64F", vr_Sub64F,
                       VR_VEC_LLO);
      break;

    case Iop_Sub64Fx2:
      vr_countOp (sb, VR_OP_SUB, VR_PREC_DBL, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;

      // - Single precision
    case Iop_SubF32: // Scalar
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Sub32F0x4: // 128b vector, lowest-lane-only
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_LLO);
      vr_replaceBinop (sb, stmt, expr, Ity_I32,
                       "vr_Sub32F", vr_Sub32F,
                       VR_VEC_LLO);
      break;

    case Iop_Sub32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;


      // Multiplication

      // - Double precision

    case Iop_MulF64: // Scalar
      vr_countOp (sb, VR_OP_MUL, VR_PREC_DBL, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Mul64F0x2: // 128b vector, lowest-lane-only
      vr_countOp (sb, VR_OP_MUL, VR_PREC_DBL, VR_VEC_LLO);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Mul64Fx2: // 128b vector, 2 lanes
      vr_countOp (sb, VR_OP_MUL, VR_PREC_DBL, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;

      // - Single precision

    case Iop_MulF32: // Scalar
      vr_countOp (sb, VR_OP_MUL, VR_PREC_FLT, VR_VEC_SCAL);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Mul32F0x4: // 128b vector, lowest-lane-only
      vr_countOp (sb, VR_OP_MUL, VR_PREC_FLT, VR_VEC_LLO);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Mul32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_MUL, VR_PREC_FLT, VR_VEC_FULL);
      addStmtToIRSB (sb, stmt);
      break;


      //   Other FP operations
    case Iop_Add32Fx2:
    case Iop_Sub32Fx2:

    case Iop_DivF64:
    case Iop_Div64F0x2:
    case Iop_Div64Fx2:
    case Iop_DivF32:
    case Iop_Div32F0x4:
    case Iop_Div32Fx4:

      VG_(printf) ("Uncounted FP operation: ");
      ppIRStmt (stmt);
      VG_(printf) ("\n");

    default:
      addStmtToIRSB (sb, stmt);
      break;
    }

}

static void vr_instrumentExpr (IRSB* sb, IRStmt* stmt, IRExpr* expr) {
  switch (expr->tag) {
  case Iex_Unop:
    vr_instrumentOp (sb, stmt, expr, expr->Iex.Unop.op);
    break;
  case Iex_Binop:
    vr_instrumentOp (sb, stmt, expr, expr->Iex.Binop.op);
    break;
  case Iex_Triop:
    vr_instrumentOp (sb, stmt, expr, expr->Iex.Triop.details->op);
    break;
  default:
    addStmtToIRSB (sb, stmt);
    break;
  }
}

static
IRSB* vr_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout,
                      VexGuestExtents* vge,
                      VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  Int i;
  IRSB* sbOut;

  if (!vr_instrument_state) {
    return sbIn;
  }

  sbOut = deepCopyIRSBExceptStmts(sbIn);

  for (i=0 ; i<sbIn->stmts_used ; ++i) {
    IRStmt* st = sbIn->stmts[i];

    switch (st->tag) {
    case Ist_WrTmp:
      vr_instrumentExpr (sbOut, st, st->Ist.WrTmp.data);
      break;
    default:
      addStmtToIRSB (sbOut, sbIn->stmts[i]);
    }
  }

  if (sbIn->jumpkind == Ijk_ClientReq) {
    ULong addr;

    tl_assert (sbIn->next->tag == Iex_Const);
    addr = sbIn->next->Iex.Const.con->Ico.U64;

    addStmtToIRSB
      (sbOut,
       IRStmt_Dirty
       ( unsafeIRDirty_0_N ( 1, "vr_set_deterministic_section",
                             VG_(fnptr_to_fnentry)(&vr_set_deterministic_section),
                             mkIRExprVec_1 (mkIRExpr_HWord (addr)))));
  }
  return sbOut;
}

static void vr_fini(Int exitcode)
{
  vr_fpOpsFini ();
  vr_ppOpCount ();
}

static void vr_post_clo_init(void)
{
   vr_fpOpsInit(vr_clo.roundingMode);
}

static void vr_pre_clo_init(void)
{
   VG_(details_name)            ("Verrou");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("Check floating-point rounding errors");
   VG_(details_copyright_author)(
      "Copyright (C) 2014, F. Fevotte & B. Lathuiliere.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (vr_post_clo_init,
                                 vr_instrument,
                                 vr_fini);

   VG_(needs_command_line_options)(vr_process_clo,
                                   vr_print_usage,
                                   vr_print_debug_usage);

   VG_(needs_client_requests)(vr_handle_client_request);

   vr_clo_defaults();
}

VG_DETERMINE_INTERFACE_VERSION(vr_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
