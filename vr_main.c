
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
#include "pub_tool_stacktrace.h"
#include "pub_tool_threadstate.h"

#include "verrou.h"
#include "vr_fpOps.h"
//#include <fenv.h>
#include "float.h"
//#pragma STDC FENV_ACCESS ON

/* * Global features
 */

/* ** Start-stop instrumentation
 */
typedef enum {
  VR_INSTR_OFF,
  VR_INSTR_ON,
  VR_INSTR
} Vr_Instr;

Vr_Instr vr_instrument_state = VR_INSTR_ON;
Bool vr_verbose= False;


// Black magic from callgrind
extern void VG_(discard_translations) ( Addr64 start, ULong range, const HChar* who );

static void vr_set_instrument_state (const HChar* reason, Vr_Instr state) {
  if (vr_instrument_state == state) {
    if(vr_verbose){
      VG_(message)(Vg_DebugMsg,"%s: instrumentation already %s\n",
		   reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
    }
    
    return;
  }

  vr_instrument_state = state;
  if(vr_instrument_state == VR_INSTR_ON){
    vr_beginInstrumentation();
  }else{
    vr_endInstrumentation();
  }

  
  if(vr_verbose){
    VG_(message)(Vg_DebugMsg, "%s: instrumentation switched %s\n",
		 reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
  }
  // Discard cached translations
  //  VG_(discard_translations)( (Addr64)0x1000, (ULong) ~0xfffl, "verrou");
}

/* ** Enter/leave deterministic section
 */

static void vr_deterministic_section_name (unsigned int level,
                                           HChar * name,
                                           unsigned int len)
{
  Addr ips[8];
  HChar fnname[256];
  HChar filename[256];
  UInt  linenum;
  Addr  addr;

  VG_(get_StackTrace)(VG_(get_running_tid)(),
                      ips, 8,
                      NULL, NULL,
                      0);
  addr = ips[level];

  fnname[0] = 0;
  VG_(get_fnname)(addr, fnname, 256);

  filename[0] = 0;
  VG_(get_filename_linenum)(addr,
                            filename, 256,
                            NULL,     0,
                            NULL,
                            &linenum);
  VG_(snprintf)(name, len,
                "%s (%s:%d)", fnname, filename, linenum);
}

static unsigned int vr_deterministic_section_hash (HChar const*const name)
{
  unsigned int hash = VG_(getpid)();
  int i = 0;
  while (name[i] != 0) {
    hash += i * name[i];
    ++i;
  }
  return hash;
}

static void vr_start_deterministic_section (unsigned int level) {
  HChar name[256];
  unsigned int hash;

  vr_deterministic_section_name (level, name, 256);

  hash = vr_deterministic_section_hash (name);
  vr_fpOpsSeed (hash);

  VG_(message)(Vg_DebugMsg, "Entering deterministic section %d: %s\n",
               hash, name);
}

static void vr_stop_deterministic_section (unsigned int level) {
  HChar name[256];
  vr_deterministic_section_name (level, name, 256);

  VG_(message)(Vg_DebugMsg, "Leaving deterministic section: %s\n",
               name);
  vr_fpOpsRandom ();
}



typedef enum {
  VR_OP_ADD,    // Addition
  VR_OP_SUB,    // Subtraction
  VR_OP_MUL,    // Multiplication
  VR_OP_DIV,    // Division
  VR_OP_MADD,   // FMA ADD
  VR_OP_MSUB,   // FMA SUB
  VR_OP
} Vr_Op;

/* ** Command-line options
 */
typedef struct _vr_CLO vr_CLO;
struct _vr_CLO {
  enum vr_RoundingMode roundingMode;
};
vr_CLO vr_clo;

Bool vr_count= True;
Bool vr_instr_op[VR_OP];
Bool vr_instr_scalar=False;

static Bool vr_process_clo (const HChar *arg) {
  Bool bool_val;
  //Option --rounding-mode=
  if      (VG_XACT_CLO (arg, "--rounding-mode=random",
                        vr_clo.roundingMode, VR_RANDOM)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=average",
                        vr_clo.roundingMode, VR_AVERAGE)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=nearest",
                        vr_clo.roundingMode, VR_NEAREST)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=upward",
                        vr_clo.roundingMode, VR_UPWARD)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=downward",
                        vr_clo.roundingMode, VR_DOWNWARD)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=toward_zero",
                        vr_clo.roundingMode, VR_ZERO)) {}

  //Options to choose op to instrument
  else if (VG_XACT_CLO (arg, "--vr-instr=add",
                        vr_instr_op[VR_OP_ADD] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=sub",
                        vr_instr_op[VR_OP_SUB] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mul",
                        vr_instr_op[VR_OP_MUL] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=div",
                        vr_instr_op[VR_OP_DIV] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mAdd",
                        vr_instr_op[VR_OP_MADD] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mSub",
                        vr_instr_op[VR_OP_MSUB] , True)) {}
  
  //Options to choose op to instrument
  else if (VG_BOOL_CLO (arg, "--vr-instr-scalar", bool_val)) {
    vr_instr_scalar= bool_val;
  }
  
  //Option --verrou-verbose (to avoid verbose of valgrind)
  else if (VG_BOOL_CLO (arg, "--verrou-verbose", bool_val)) {
    vr_verbose = bool_val;
  }

  //Option --count-op
  else if (VG_BOOL_CLO (arg, "--count-op", bool_val)) {
    vr_count = bool_val;
  }

  // Instrumentation at start
  else if (VG_BOOL_CLO (arg, "--instr-atstart", bool_val)) {
    vr_instrument_state = bool_val ? VR_INSTR_ON : VR_INSTR_OFF;
  }

  // Unknown option
  else{
    return False;
  }

  return True;
}

static void vr_clo_defaults (void) {
  vr_clo.roundingMode = VR_NEAREST;
  int opIt;
  for(opIt=0; opIt< VR_OP;opIt++){
    vr_instr_op[opIt]=False;
  }
}

static void vr_print_usage (void) {
  VG_(printf)("Verrou Options: \n");
  VG_(printf)("    Rounding mode selection \n");
  VG_(printf)("        --rounding-mode=random\n");
  VG_(printf)("        --rounding-mode=average\n");
  VG_(printf)("        --rounding-mode=nearest\n");
  VG_(printf)("        --rounding-mode=upward\n");
  VG_(printf)("        --rounding-mode=downward\n");
  VG_(printf)("        --rounding-mode=toward_zero\n");
  VG_(printf)("    Instrumented  operations  selection \n");  
  VG_(printf)("         --vr-instr=add\n");
  VG_(printf)("         --vr-instr=sub\n");
  VG_(printf)("         --vr-instr=mul\n");
  VG_(printf)("         --vr-instr=div\n");
  VG_(printf)("         --vr-instr=mAdd\n");
  VG_(printf)("         --vr-instr=mSub\n");


  VG_(printf)("    Other options \n");
  VG_(printf)("        --vr-instr-scalar=[yes|NO] : instrument scalar operation (x387)\n");
  VG_(printf)("        --verrou-verbose=[yes|NO] : print each instrumentation switch\n");
  VG_(printf)("        --count-op=[yes|NO] : count floating point operations\n");
  VG_(printf)("        --instr-atstart=[YES|no] : intrumenation from the start (useful used with client request)\n");


}

static void vr_print_debug_usage (void) {
  vr_print_usage();
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
    vr_start_deterministic_section (args[1]);
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__STOP_DETERMINISTIC:
    vr_stop_deterministic_section (args[1]);
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

static const char* vr_ppOp (Vr_Op op) {
  switch (op) {
  case VR_OP_ADD:
    return "add";
  case VR_OP_SUB:
    return "sub";
  case VR_OP_MUL:
    return "mul";
  case VR_OP_DIV:
    return "div";
  case VR_OP_MADD:
    return "mAdd";
  case VR_OP_MSUB:
    return "mSub";

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
  VR_VEC_FULL2,  // Vector operation
  VR_VEC_FULL4,  // Vector operation
  VR_VEC
} Vr_Vec;

static const char* vr_ppVec (Vr_Vec vec) {
  switch (vec) {
  case VR_VEC_SCAL:
    return "scal";
  case VR_VEC_LLO:
    return "llo ";
  case VR_VEC_FULL2:
    return "vec2 ";
  case VR_VEC_FULL4:
    return "vec4 ";
  default:
    return "unknown";
  }
}

/* ** Counter handling
 */

static ULong vr_opCount[VR_OP][VR_PREC][VR_VEC][VR_INSTR];
static VG_REGPARM(2) void vr_incOpCount (ULong* counter, Long increment) {
  counter[vr_instrument_state] += increment;
}

static VG_REGPARM(2) void vr_incUnstrumentedOpCount (ULong* counter, Long increment) {
  counter[VR_INSTR_OFF] += increment; 
}

static void vr_countOp (IRSB* sb, Vr_Op op, Vr_Prec prec, Vr_Vec vec) {
  if(!vr_count){
    return;
  }
    
  IRExpr** argv;
  IRDirty* di;
  int increment = 1;
  if (vec == VR_VEC_FULL2) {
    increment =2;
  }
  if(vec == VR_VEC_FULL4) {
    increment =4;
  }
   
  if( vr_instr_op[op] && ( (vr_instr_scalar || !vec==VR_VEC_SCAL ))&& (vec!=VR_VEC_FULL4) ){
    argv = mkIRExprVec_2 (mkIRExpr_HWord ((HWord)&vr_opCount[op][prec][vec]),
			  mkIRExpr_HWord (increment));
    di = unsafeIRDirty_0_N( 2,
                          "vr_incOpCount",
			    VG_(fnptr_to_fnentry)( &vr_incOpCount ),
			    argv);

  }else{
    argv = mkIRExprVec_2 (mkIRExpr_HWord ((HWord)&vr_opCount[op][prec][vec]),
			  mkIRExpr_HWord (increment));

    di = unsafeIRDirty_0_N( 2,
			    "vr_incUnstrumentedOpCount",
			    VG_(fnptr_to_fnentry)( &vr_incUnstrumentedOpCount ),
			    argv);

  }

  addStmtToIRSB (sb, IRStmt_Dirty (di));
}

static unsigned int vr_frac (ULong a, ULong b) {
  unsigned int q = (100*a)/(a+b);
  if (100*a - (a+b)*q > (a+b)/2) {q++;}
  return q;
}

static void vr_ppOpCount (void) {
  if(!vr_count)return ;
  Vr_Op op;
  Vr_Prec prec;
  Vr_Vec vec;

  VG_(umsg)(" ---------------------------------------------------------------------\n");
  VG_(umsg)(" Operation                            Instruction count\n");
  VG_(umsg)("  `- Precision\n");
  VG_(umsg)("      `- Vectorization          Total             Instrumented\n");
  VG_(umsg)(" ---------------------------------------------------------------------\n");
  for (op = 0 ; op<VR_OP ; ++op) {
    ULong countOp[VR_INSTR];
    countOp[VR_INSTR_ON]  = 0;
    countOp[VR_INSTR_OFF] = 0;

    for (prec = 0 ; prec < VR_PREC ; ++prec) {
      for (vec = 0 ; vec < VR_VEC ; ++vec) {
        countOp[VR_INSTR_ON]  += vr_opCount[op][prec][vec][VR_INSTR_ON];
        countOp[VR_INSTR_OFF] += vr_opCount[op][prec][vec][VR_INSTR_OFF];
      }
    }

    if (countOp[VR_INSTR_ON] + countOp[VR_INSTR_OFF] > 0) {
      VG_(umsg)(" %6s       %15llu          %15llu          (%3u%%)\n",
                vr_ppOp(op),
                countOp[VR_INSTR_ON] + countOp[VR_INSTR_OFF],
                countOp[VR_INSTR_ON],
                vr_frac (countOp[VR_INSTR_ON], countOp[VR_INSTR_OFF]));

      for (prec = 0 ; prec<VR_PREC ; ++prec) {
        ULong countPrec[VR_INSTR];
        countPrec[VR_INSTR_ON]  = 0;
        countPrec[VR_INSTR_OFF] = 0;

        for (vec = 0 ; vec<VR_VEC ; ++vec) {
          countPrec[VR_INSTR_ON]  += vr_opCount[op][prec][vec][VR_INSTR_ON];
          countPrec[VR_INSTR_OFF] += vr_opCount[op][prec][vec][VR_INSTR_OFF];
        }

        if (countPrec[VR_INSTR_ON] + countPrec[VR_INSTR_OFF] > 0) {
          VG_(umsg)("  `- %6s       %15llu          %15llu      (%3u%%)\n",
                    vr_ppPrec(prec),
                    countPrec[VR_INSTR_ON] + countPrec[VR_INSTR_OFF],
                    countPrec[VR_INSTR_ON],
                    vr_frac (countPrec[VR_INSTR_ON], countPrec[VR_INSTR_OFF]));

          for (vec = 0 ; vec<VR_VEC ; ++vec) {
            ULong * count = vr_opCount[op][prec][vec];
            if (count[VR_INSTR_ON] + count[VR_INSTR_OFF] > 0) {
              VG_(umsg)("      `- %6s       %15llu          %15llu  (%3u%%)\n",
                        vr_ppVec(vec),
                        vr_opCount[op][prec][vec][VR_INSTR_ON] + vr_opCount[op][prec][vec][VR_INSTR_OFF],
                        vr_opCount[op][prec][vec][VR_INSTR_ON],
                        vr_frac (vr_opCount[op][prec][vec][VR_INSTR_ON], vr_opCount[op][prec][vec][VR_INSTR_OFF]));
            }
          }
        }
      }
      VG_(umsg)(" ---------------------------------------------------------------------\n");
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


static VG_REGPARM(2) Long vr_Mul64F (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res = vr_MulDouble (*arg1, *arg2);
  Long *c = (Long*)(&res);
  return *c;
}

static VG_REGPARM(2) Long vr_Div64F (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res = vr_DivDouble (*arg1, *arg2);
  Long *c = (Long*)(&res);
  return *c;
}


static VG_REGPARM(3) Long vr_MAdd64F (Long a, Long b, Long c) {  
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res =vr_MAddDouble (*arg1, *arg2, *arg3);
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int vr_MAdd32F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res =vr_MAddFloat (*arg1, *arg2, *arg3);
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}


static VG_REGPARM(3) Long vr_MSub64F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res =vr_MAddDouble (*arg1, *arg2, -*arg3);
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int vr_MSub32F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res =vr_MAddFloat (*arg1, *arg2, -*arg3);
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}


static VG_REGPARM(2) Int vr_Mul32F (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res = vr_MulFloat (*arg1, *arg2);
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(2) Int vr_Div32F (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res = vr_DivFloat (*arg1, *arg2);
  Int *c = (Int*)(&res);
  return *c;
}



/* ** Code instrumentation
 */

/* *** Helpers
 */

/* Return the Lowest Lane of a given packed temporary register */
static IRExpr* vr_getLLFloat (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I32);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_V128to32, expr)));
  return IRExpr_RdTmp(tmp);
}
/* Return the Lowest Lane of a given packed temporary register */
static IRExpr* vr_getLLDouble (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I64);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_V128to64, expr)));
  return IRExpr_RdTmp(tmp);
 }

/* Return the Highest Lane of a given packed temporary register */
static IRExpr* vr_getHLDouble (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I64);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_V128HIto64, expr)));
  return IRExpr_RdTmp(tmp);
 }






static IRExpr* vr_I32toI64 (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I64);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_32Uto64, expr)));
  return IRExpr_RdTmp (tmp);
}

static IRExpr* vr_I64toI32 (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I32);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_64to32, expr)));
  return IRExpr_RdTmp (tmp);
}



static IRExpr* vr_F64toI64 (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I64);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_ReinterpF64asI64, expr)));
  return IRExpr_RdTmp (tmp);
}


static IRExpr* vr_F32toI64 (IRSB* sb, IRExpr* expr) {
  IRTemp tmp = newIRTemp (sb->tyenv, Ity_I32);
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp, IRExpr_Unop (Iop_ReinterpF32asI32, expr)));
  return vr_I32toI64(sb, IRExpr_RdTmp (tmp));
}



/* Replace a given binary operation by a call to a function
 */


static void vr_replaceBinFpOp (IRSB* sb, IRStmt* stmt, IRExpr* expr, 
			       const HChar* functionName, void* function,
			       Vr_Op op,
			       Vr_Prec prec,
			       Vr_Vec vec) {
  //instrumentation to count operation
  vr_countOp (sb,  op, prec,vec);

  if(vr_verbose && vec==VR_VEC_SCAL){
    VG_(printf) ("Scalar Instruction : ");
    ppIRStmt (stmt);
    VG_(printf) ("\n");
    VG_(get_and_pp_StackTrace)(VG_(get_running_tid)(), VG_(clo_backtrace_size));
    VG_(printf) ("\n");
  }
  
  if(!vr_instr_op[op] ) {
    addStmtToIRSB (sb, stmt);
    return;
  }
  if(!vr_instr_scalar && vec==VR_VEC_SCAL) {
    addStmtToIRSB (sb, stmt);
    return;
  }


  //convertion before call
  IRTemp res;   
  IRExpr * arg1;
  IRExpr * arg2;
  if (vec == VR_VEC_LLO) {
    arg1 = expr->Iex.Binop.arg1;
    arg2 = expr->Iex.Binop.arg2;
    if (prec==VR_PREC_FLT) {
      arg1 = vr_getLLFloat (sb, arg1);
      arg2 = vr_getLLFloat (sb, arg2);
      arg1 = vr_I32toI64 (sb, arg1);
      arg2 = vr_I32toI64 (sb, arg2);
      res= newIRTemp (sb->tyenv, Ity_I32);
    }
    if (prec==VR_PREC_DBL) {
      arg1 = vr_getLLDouble (sb, arg1);
      arg2 = vr_getLLDouble (sb, arg2);
      res= newIRTemp (sb->tyenv, Ity_I64);
    }
  }

  if(vec== VR_VEC_SCAL){
    arg1 = expr->Iex.Triop.details->arg2;
    //shift arg is not a bug :  IRRoundingMode(I32) x F64 x F64 -> F64 */ 
    arg2 = expr->Iex.Triop.details->arg3;
    
    if (prec==VR_PREC_FLT) {
      arg1=vr_F32toI64(sb,arg1);
      arg2=vr_F32toI64(sb,arg2);    
      res= newIRTemp (sb->tyenv, Ity_I32);
    }
    if (prec==VR_PREC_DBL) {
      arg1=vr_F64toI64(sb,arg1);
      arg2=vr_F64toI64(sb,arg2);
      res= newIRTemp (sb->tyenv, Ity_I64);
    }
  }

  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1, arg2))));

  //conversion after call
  if (vec == VR_VEC_LLO) {
    IROp opReg;
    if (prec==VR_PREC_FLT) opReg = Iop_32UtoV128;
    if (prec==VR_PREC_DBL) opReg = Iop_64UtoV128;
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                     IRExpr_Unop (opReg, IRExpr_RdTmp(res))));
  }
  if (vec == VR_VEC_SCAL) {
    if(prec==VR_PREC_FLT){   
      IRExpr* conv=vr_I64toI32(sb, IRExpr_RdTmp(res ));
      addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				       IRExpr_Unop (Iop_ReinterpI32asF32, conv )));
    }
    if(prec==VR_PREC_DBL){   
      addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				       IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
    }
  }
}












static void vr_replaceBinFull2Op (IRSB* sb, IRStmt* stmt, IRExpr* expr, 
			       const HChar* functionName, void* function,
			       Vr_Op op,
			       Vr_Prec prec,
			       Vr_Vec vec) {
  //instrumentation to count operation
  vr_countOp (sb,  op, prec,vec);

  if(!vr_instr_op[op] ) {
    addStmtToIRSB (sb, stmt);
    return;
  }

  if(vec!=VR_VEC_FULL2){
    VG_(tool_panic) ( "vec != VECT FULL2 in vr_replaceBinFull2Op  \n");
  }
  
  if (prec==VR_PREC_FLT) {
    VG_(tool_panic) ( "VECT FULL2 and FLOAT not compatible \n");
  }

  //convertion before call
  IRExpr * arg1 = expr->Iex.Triop.details->arg2;
  IRExpr * arg2 = expr->Iex.Triop.details->arg3;

  
  IRExpr *arg1Lo=vr_getLLDouble (sb, arg1);
  IRExpr *arg1Hi=vr_getHLDouble (sb, arg1);
  IRExpr *arg2Lo=vr_getLLDouble (sb, arg2);
  IRExpr *arg2Hi=vr_getHLDouble (sb, arg2);
    
  IRTemp resLo= newIRTemp (sb->tyenv, Ity_I64);
  IRTemp resHi= newIRTemp (sb->tyenv, Ity_I64);

  
  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (resLo, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1Lo, arg2Lo))));

  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (resHi, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1Hi, arg2Hi))));


  //conversion after call
 
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				   IRExpr_Binop (Iop_64HLtoV128,  
						 IRExpr_RdTmp(resHi),
						 IRExpr_RdTmp(resLo))
				   ));
}




static void vr_replaceFMA (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			   const HChar* functionName, void* function,
			   Vr_Op   Op,
			   Vr_Prec Prec) {
  vr_countOp (sb,  Op, Prec, VR_VEC_LLO);
  if(!vr_instr_op[Op] ) {
    addStmtToIRSB (sb, stmt);
    return;
  }

  
#ifdef USE_VERROU_FMA
  //  IRExpr * arg1 = expr->Iex.Qop.details->arg1; Rounding mode
  IRExpr * arg2 = expr->Iex.Qop.details->arg2;
  IRExpr * arg3 = expr->Iex.Qop.details->arg3;
  IRExpr * arg4 = expr->Iex.Qop.details->arg4;
  IRTemp res = newIRTemp (sb->tyenv, Ity_I64);
  if(Prec== VR_PREC_DBL){     
    arg2=vr_F64toI64(sb,arg2);
    arg3=vr_F64toI64(sb,arg3);
    arg4=vr_F64toI64(sb,arg4);

  }
  if(Prec==VR_PREC_FLT){
    arg2=vr_F32toI64(sb,arg2);
    arg3=vr_F32toI64(sb,arg3);
    arg4=vr_F32toI64(sb,arg4);    
  }

  IRDirty* id= unsafeIRDirty_1_N (res, 3,
				  functionName, VG_(fnptr_to_fnentry)(function),
				  mkIRExprVec_3 (arg2, arg3,arg4));

  addStmtToIRSB (sb,IRStmt_Dirty(id));

  

  if(Prec==VR_PREC_FLT){   
    IRExpr* conv=vr_I64toI32(sb, IRExpr_RdTmp(res ));
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
    				     IRExpr_Unop (Iop_ReinterpI32asF32, conv )));
  }
  if(Prec==VR_PREC_DBL){   
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
  }
#else //USE_VERROU_FMA
  //should not happed
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif //USE_VERROU_FMA
}


static void vr_instrumentOp (IRSB* sb, IRStmt* stmt, IRExpr * expr, IROp op) {
    switch (op) {

      // Addition

      // - Double precision
    case Iop_AddF64: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Add64F", vr_Add64F, VR_OP_ADD,VR_PREC_DBL,VR_VEC_SCAL);
      break;

    case Iop_Add64F0x2: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Add64F", vr_Add64F,VR_OP_ADD, VR_PREC_DBL,VR_VEC_LLO);
      break;

    case Iop_Add64Fx2: // 128b vector, 2 lanes
      vr_replaceBinFull2Op (sb, stmt, expr,"vr_Add64F", vr_Add64F,VR_OP_ADD, VR_PREC_DBL,VR_VEC_FULL2);
      break;

      // - Single precision
    case Iop_AddF32: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Add32F", vr_Add32F, VR_OP_ADD,VR_PREC_FLT,VR_VEC_SCAL);
      break;

    case Iop_Add32F0x4: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Add32F", vr_Add32F, VR_OP_ADD,VR_PREC_FLT,VR_VEC_LLO);
      break;

    case Iop_Add32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;


      // Subtraction

      // - Double precision
    case Iop_SubF64: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Sub64F", vr_Sub64F, VR_OP_SUB,VR_PREC_DBL,VR_VEC_SCAL);
      break;

    case Iop_Sub64F0x2: // 128b vector, lowest-lane only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Sub64F", vr_Sub64F, VR_OP_SUB, VR_PREC_DBL,VR_VEC_LLO);
      break;

    case Iop_Sub64Fx2:
      vr_replaceBinFull2Op (sb, stmt, expr,"vr_Sub64F", vr_Sub64F, VR_OP_SUB, VR_PREC_DBL,VR_VEC_FULL2);
      break;

      // - Single precision
    case Iop_SubF32: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Sub32F", vr_Sub32F, VR_OP_SUB,VR_PREC_FLT,VR_VEC_SCAL);
      break;

    case Iop_Sub32F0x4: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Sub32F", vr_Sub32F, VR_OP_SUB,VR_PREC_FLT,VR_VEC_LLO);
      break;

    case Iop_Sub32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;


      // Multiplication

      // - Double precision
    case Iop_MulF64: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Mul64F", vr_Mul64F, VR_OP_MUL,VR_PREC_DBL,VR_VEC_SCAL);
      break;
    case Iop_Mul64F0x2: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Mul64F", vr_Mul64F, VR_OP_MUL,VR_PREC_DBL,VR_VEC_LLO);
      break;
    case Iop_Mul64Fx2: // 128b vector, 2 lanes
      vr_replaceBinFull2Op (sb, stmt, expr,"vr_Mul64F", vr_Mul64F, VR_OP_MUL,VR_PREC_DBL,VR_VEC_FULL2);
      break;

      // - Single precision
    case Iop_MulF32: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Mul32F", vr_Mul32F,VR_OP_MUL, VR_PREC_FLT,VR_VEC_SCAL);
      break;
    case Iop_Mul32F0x4: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Mul32F", vr_Mul32F, VR_OP_MUL,VR_PREC_FLT,VR_VEC_LLO);
      break;
    case Iop_Mul32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_MUL, VR_PREC_FLT, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;

      
    case Iop_DivF32:
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Div32F", vr_Div32F, VR_OP_DIV,VR_PREC_FLT,VR_VEC_SCAL);
      break;

    case Iop_Div32F0x4: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Div32F", vr_Div32F, VR_OP_DIV,VR_PREC_FLT,VR_VEC_LLO);
      break;

    case Iop_Div32Fx4: // 128b vector, 4 lanes
      vr_countOp (sb, VR_OP_DIV, VR_PREC_FLT, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;


    case Iop_DivF64: // Scalar
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Div64F", vr_Div64F, VR_OP_DIV,VR_PREC_DBL,VR_VEC_SCAL);
      break;
    case Iop_Div64F0x2: // 128b vector, lowest-lane-only
      vr_replaceBinFpOp (sb, stmt, expr,"vr_Div64F", vr_Div64F,VR_OP_DIV, VR_PREC_DBL,VR_VEC_LLO);
      break;
    case Iop_Div64Fx2: // 128b vector, 2 lanes
      vr_replaceBinFull2Op(sb, stmt, expr,"vr_Div64F", vr_Div64F,VR_OP_DIV, VR_PREC_DBL,VR_VEC_FULL2);
      break;


    case Iop_MAddF32:
      vr_replaceFMA (sb, stmt, expr,"vr_MAdd32F", vr_MAdd32F, VR_OP_MADD, VR_PREC_FLT);
      break;
    case Iop_MSubF32:
      vr_replaceFMA (sb, stmt, expr,"vr_MSub32F", vr_MSub32F, VR_OP_MSUB, VR_PREC_FLT);
      break;
    case Iop_MAddF64:
      vr_replaceFMA (sb, stmt, expr,"vr_MAdd64F", vr_MAdd64F, VR_OP_MADD, VR_PREC_DBL);
      break;
    case Iop_MSubF64:
      vr_replaceFMA (sb, stmt, expr,"vr_MSub64F", vr_MSub64F,VR_OP_MSUB,  VR_PREC_DBL);
      break;


      //   Other FP operations
    case Iop_Add32Fx2:
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL2);
      addStmtToIRSB (sb, stmt);
      break;

      
    case Iop_Sub32Fx2:
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL2);
      addStmtToIRSB (sb, stmt);
      break;

      
      /*AVX double*/
    case Iop_Add64Fx4:
      vr_countOp (sb, VR_OP_ADD, VR_PREC_DBL, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;

      
    case Iop_Sub64Fx4:
      vr_countOp (sb, VR_OP_SUB, VR_PREC_DBL, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Mul64Fx4:
      vr_countOp (sb, VR_OP_MUL, VR_PREC_DBL, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_Div64Fx4:
      vr_countOp (sb, VR_OP_DIV, VR_PREC_DBL, VR_VEC_FULL4);
      addStmtToIRSB (sb, stmt);
      break;

      
      //operation with 64bit register with 32bit rounding
    case Iop_AddF64r32:
    case Iop_SubF64r32:
    case Iop_MulF64r32:
    case Iop_DivF64r32: 
    case Iop_MAddF64r32:
    case Iop_MSubF64r32:

      //operation wit 128bit
    case Iop_AddF128:
    case Iop_SubF128:
    case Iop_MulF128:
    case Iop_DivF128:

    case Iop_SqrtF128:
    case Iop_SqrtF64:
    case Iop_SqrtF32:
    case Iop_Sqrt32Fx4:
    case Iop_Sqrt64Fx2:
    case  Iop_AtanF64:       /* FPATAN,  arctan(arg1/arg2)       */
    case  Iop_Yl2xF64:       /* FYL2X,   arg1 * log2(arg2)       */
    case  Iop_Yl2xp1F64:     /* FYL2XP1, arg1 * log2(arg2+1.0)   */
    case  Iop_PRemF64:       /* FPREM,   non-IEEE remainder(arg1/arg2)    */
    case  Iop_PRemC3210F64:  /* C3210 flags resulting from FPREM: :: I32 */
    case  Iop_PRem1F64:      /* FPREM1,  IEEE remainder(arg1/arg2)    */
    case  Iop_PRem1C3210F64: /* C3210 flags resulting from FPREM1, :: I32 */
    case  Iop_ScaleF64:      /* FSCALE,  arg1 * (2^RoundTowardsZero(arg2)) */
    case  Iop_SinF64:    /* FSIN */
    case  Iop_CosF64:    /* FCOS */
    case  Iop_TanF64:    /* FTAN */
    case  Iop_2xm1F64:   /* (2^arg - 1.0) */

    case Iop_RSqrtEst5GoodF64: /* reciprocal square root estimate, 5 good bits */

    case Iop_RecipStep32Fx4:
    case Iop_RSqrtEst32Fx4:
    case Iop_RSqrtStep32Fx4:
    case Iop_RecipEst32F0x4:
    case Iop_Sqrt32F0x4:
    case Iop_RSqrtEst32F0x4:

      /*AVX*/
    case Iop_Sqrt32Fx8:
    case Iop_Sqrt64Fx4:
    case Iop_RSqrtEst32Fx8:
    case Iop_RecipEst32Fx8:
	

    case Iop_Add32Fx8:
    case Iop_Sub32Fx8:
    case Iop_Mul32Fx8:
    case Iop_Div32Fx8:


	//    case Iop_DivF64:
      //    case Iop_Div64F0x2:
      //    case Iop_Div64Fx2:

      VG_(printf) ("Uncounted FP operation: ");
      ppIRStmt (stmt);
      VG_(printf) ("\n");
      if(vr_verbose){
	VG_(get_and_pp_StackTrace)(VG_(get_running_tid)(), VG_(clo_backtrace_size));
      }

    default:
      //      ppIRStmt (stmt);
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
  case Iex_Qop:
    vr_instrumentOp (sb, stmt, expr, expr->Iex.Qop.details->op);
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

  //  if (!vr_instrument_state) {
  //    return sbIn;
  //  }

  { // Don't instrument code in libm functions
    Addr  ips[8];
    HChar fnname[256];
    HChar filename[256];
    UInt  linenum;
    Addr  addr;

    VG_(get_StackTrace)(VG_(get_running_tid)(),
                        ips, 8,
                        NULL, NULL,
                        0);
    addr = ips[0];

    fnname[0] = 0;
    VG_(get_fnname)(addr, fnname, 256);

    filename[0] = 0;
    VG_(get_filename_linenum)(addr,
                              filename, 256,
                              NULL,     0,
                              NULL,
                            &linenum);

    // Uncomment this line to list all functions (useful to generate the suppression list)
    // VG_(printf)("vrSym %s (%s:%d)\n", fnname, filename, linenum);

    // WARNING: This list MUST be terminated by ""
    const char *suppress[] = {"__exp1",
                              "__ieee754_exp",
                              "__ieee754_expf",
                              "__ieee754_log",
                              "__ieee754_logf",
                              "__ieee754_pow",
                              "__ieee754_powf",
                              "__ieee754_rem_pio2f",
                              "__kernel_cosf",
                              "__kernel_sinf",
                              "__kernel_tanf",
                              "__signArctan",
                              "atan",
                              "atanf",
                              "cos",
                              "cosf",
                              "exp",
                              "expf",
                              "fesetenv",
                              "fesetround",
                              "log",
                              "logf",
                              "pow",
                              "powf",
                              "sin",
                              "sincos",
                              "sincosf",
                              "sinf",
                              "tan",
                              "tanf",
                              ""};
    for (i=0 ; suppress[i][0] != 0 ; ++i) {
      if (VG_(strcmp)(fnname, suppress[i]) == 0) {
        return sbIn;
      }
    }
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

  return sbOut;
}

static void vr_fini(Int exitcode)
{
  vr_fpOpsFini ();
  vr_ppOpCount ();
}

static void vr_post_clo_init(void)
{
   vr_fpOpsInit(vr_clo.roundingMode, VG_(getpid)());

   /*If no operation selected the default is all*/
   Bool someThingInstr=False;
   int opIt;
   for(opIt=0; opIt< VR_OP ;opIt++){
     if(vr_instr_op[opIt]) someThingInstr=True;
   }
   if(!someThingInstr){
     for(opIt=0; opIt< VR_OP ;opIt++){
       vr_instr_op[opIt]=True;
     }
   }
   VG_(umsg)("Instrumented operations :\n");
   for (opIt=0; opIt< VR_OP ;opIt++){
     VG_(umsg)("\t%s : ", vr_ppOp(opIt));
     if(vr_instr_op[opIt]==True) VG_(umsg)("yes\n");
     else VG_(umsg)("no\n");
   }  
   VG_(umsg)("Instrumented scalar operations : ");
   if(vr_instr_scalar==True) VG_(umsg)("yes\n");
   else VG_(umsg)("no\n");
   
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
