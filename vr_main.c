
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

#include <fenv.h>
//#pragma STDC FENV_ACCESS ON

static void vr_post_clo_init(void)
{
}


static ULong fpOpCount;
static void incrementFpOpCount (void) {
  fpOpCount++;
}

static void vr_handleOp (const char * title1, const char * title2, IRExpr * expr, IROp op, IRSB* sb) {
    switch (op) {
      // FPU Operations

      //   Additions
      //     64bits
    case Iop_AddF64:     // Standard
    case Iop_Add64Fx2:   // SSE extension
    case Iop_Add64F0x2:  // ...lowest-lane only

      //     32bits
    case Iop_AddF32:
    case Iop_Add32Fx4:
    case Iop_Add32F0x4:

      //   Other FP operations
    case Iop_SubF32:
    case Iop_SubF64:
    case Iop_MulF32:
    case Iop_MulF64:
    case Iop_DivF32:
    case Iop_DivF64:
    case Iop_Sub32Fx4:
    case Iop_Mul32Fx4:
    case Iop_Div32Fx4:
    case Iop_Add32Fx2:
    case Iop_Sub32Fx2:
    case Iop_Sub32F0x4:
    case Iop_Mul32F0x4:
    case Iop_Div32F0x4:
    case Iop_Sub64Fx2:
    case Iop_Mul64Fx2:
    case Iop_Div64Fx2:
    case Iop_Sub64F0x2:
    case Iop_Mul64F0x2:
    case Iop_Div64F0x2:
      VG_(printf) ("%s %s FP operation: ", title1, title2);
      ppIRExpr (expr);
      VG_(printf) ("\n");

      {
        IRExpr** argv = mkIRExprVec_0();
        IRDirty* di = unsafeIRDirty_0_N( 0, "incrementFpOpCount",
                                         VG_(fnptr_to_fnentry)( &incrementFpOpCount ),
                                         argv);
        addStmtToIRSB (sb, IRStmt_Dirty (di));
      }
      break;
    default:
      /* VG_(printf) ("%s Binary INT: ", title); */
      /* ppIRExpr (expr); */
      /* VG_(printf) ("\n"); */
      break;
    }

}

static void vr_handleExpr (const char * title, IRExpr* expr, IRSB* sb) {
  switch (expr->tag) {
  case Iex_Binop:
    vr_handleOp (title, "Binop", expr, expr->Iex.Binop.op, sb);
    break;
  case Iex_Triop:
    vr_handleOp (title, "Triop", expr, expr->Iex.Triop.details->op, sb);
    break;
  default:
    /* ppIRExpr (expr); */
    /* VG_(printf) ("\n"); */
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

  sbOut = deepCopyIRSBExceptStmts(sbIn);

  for (i=0 ; i<sbIn->stmts_used ; ++i) {
    IRStmt* st = sbIn->stmts[i];

    switch (st->tag) {
    case Ist_NoOp:
    case Ist_IMark:     /* META */
    case Ist_AbiHint:   /* META */
    case Ist_PutI:
    case Ist_LoadG:
    case Ist_CAS:
    case Ist_Dirty:
    case Ist_MBE:
    case Ist_Exit:
    case Ist_Put:
    case Ist_LLSC:
      break;
    case Ist_WrTmp:
      vr_handleExpr ("WrTmp", st->Ist.WrTmp.data, sbOut);
      break;
    case Ist_Store:
      vr_handleExpr ("Store", st->Ist.Store.data, sbOut);
      break;
    case Ist_StoreG:
      vr_handleExpr ("StoreG", st->Ist.StoreG.details->data, sbOut);
      break;
    }

    addStmtToIRSB (sbOut, sbIn->stmts[i]);
  }

  return sbOut;
}

static void vr_fini(Int exitcode)
{
  VG_(umsg) ("FP operations count: %llu", fpOpCount);
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

   /* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(vr_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
