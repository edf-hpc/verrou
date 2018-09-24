
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                vr_main.c ---*/
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

#include "vr_main.h"
#include "float.h"
#include "pub_tool_libcfile.h"
//#pragma STDC FENV_ACCESS ON

Vr_State vr;
struct interflop_backend_interface_t backend;
void* backend_context;
VgFile * vr_outCancellationFile;
extern int CHECK_C;
// * Floating-point operations counter


// ** Operation categories


// *** Operation type


static const HChar* vr_ppOp (Vr_Op op) {
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
  case VR_OP_CMP:
    return "cmp";
  case VR_OP_CONV:
    return "conv";
  case VR_OP_MAX:
    return "max";
  case VR_OP_MIN:
    return "min";
  case VR_OP:
    break;
  }
  return "unknown";
}

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

static const HChar* vr_ppPrec (Vr_Prec prec) {
  switch (prec) {
  case VR_PREC_FLT:
    return "flt";
  case VR_PREC_DBL:
    return "dbl";
  case VR_PREC_DBL_TO_FLT:
    return "dbl=>flt";
  case VR_PREC_FLT_TO_DBL:
    return "flt=>dbl";
  case VR_PREC_DBL_TO_INT:
    return "dbl=>int";
  case VR_PREC_FLT_TO_INT:
    return "flt=>int";
  case VR_PREC_DBL_TO_SHT:
    return "dbl=>sht";
  case VR_PREC_FLT_TO_SHT:
    return "flt=>sht";
  case VR_PREC:
    break;
  }
  return "unknown";
}

// *** Vector operations

typedef enum {
  VR_VEC_SCAL,  // Scalar operation
  VR_VEC_LLO,   // Vector operation, lowest lane only
  VR_VEC_FULL2,  // Vector operation
  VR_VEC_FULL4,  // Vector operation
  VR_VEC_FULL8,  // Vector operation
  VR_VEC
} Vr_Vec;

static const HChar* vr_ppVec (Vr_Vec vec) {
  switch (vec) {
  case VR_VEC_SCAL:
    return "scal";
  case VR_VEC_LLO:
    return "llo ";
  case VR_VEC_FULL2:
    return "vec2 ";
  case VR_VEC_FULL4:
    return "vec4 ";
  case VR_VEC_FULL8:
    return "vec8 ";

  default:
    return "unknown";
  }
}

// ** Counter handling


static ULong vr_opCount[VR_OP][VR_PREC][VR_VEC][VR_INSTR];
static VG_REGPARM(2) void vr_incOpCount (ULong* counter, SizeT increment) {
  counter[vr.instrument] += increment;
}

static VG_REGPARM(2) void vr_incUnstrumentedOpCount (ULong* counter, SizeT increment) {
  counter[VR_INSTR_OFF] += increment;
}

static void vr_countOp (IRSB* sb, Vr_Op op, Vr_Prec prec, Vr_Vec vec, Bool instr) {
  if(!vr.count){
    return;
  }

  IRExpr** argv;
  IRDirty* di;
  SizeT increment = 1;
  if (vec == VR_VEC_FULL2) {
    increment =2;
  }
  if(vec == VR_VEC_FULL4) {
    increment =4;
  }
  if(vec == VR_VEC_FULL8) {
    increment =8;
  }

  if(instr){
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

void vr_ppOpCount (void) {
  if(!vr.count)return ;
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
      VG_(umsg)(" %-6s       %15llu          %15llu          (%3u%%)\n",
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
          VG_(umsg)("  `- %-8s     %15llu          %15llu      (%3u%%)\n",
                    vr_ppPrec(prec),
                    countPrec[VR_INSTR_ON] + countPrec[VR_INSTR_OFF],
                    countPrec[VR_INSTR_ON],
                    vr_frac (countPrec[VR_INSTR_ON], countPrec[VR_INSTR_OFF]));

          for (vec = 0 ; vec<VR_VEC ; ++vec) {
            ULong * count = vr_opCount[op][prec][vec];
            if (count[VR_INSTR_ON] + count[VR_INSTR_OFF] > 0) {
              VG_(umsg)("      `- %-6s       %15llu          %15llu  (%3u%%)\n",
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


// * Floating point operations overload


// ** Overloaded operators


#include "vr_interp_operator_impl.h"




// ** Code instrumentation


// *** Helpers


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



/* Return the Highest Lane of a given packed temporary register */
static void vr_getTabArgAVX (IRSB* sb, IRExpr* expr, IRExpr** tab) {
  IRTemp tmp0 = newIRTemp (sb->tyenv, Ity_I64);
  IRTemp tmp1 = newIRTemp (sb->tyenv, Ity_I64);
  IRTemp tmp2 = newIRTemp (sb->tyenv, Ity_I64);
  IRTemp tmp3 = newIRTemp (sb->tyenv, Ity_I64);


  addStmtToIRSB (sb, IRStmt_WrTmp (tmp0, IRExpr_Unop (Iop_V256to64_0, expr)));
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp1, IRExpr_Unop (Iop_V256to64_1, expr)));
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp2, IRExpr_Unop (Iop_V256to64_2, expr)));
  addStmtToIRSB (sb, IRStmt_WrTmp (tmp3, IRExpr_Unop (Iop_V256to64_3, expr)));


  tab[0]=IRExpr_RdTmp(tmp0);
  tab[1]=IRExpr_RdTmp(tmp1);
  tab[2]=IRExpr_RdTmp(tmp2);
  tab[3]=IRExpr_RdTmp(tmp3);      
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


/* Get the operation from an expression
   return False if the expression is not an operation
 */
static Bool vr_getOp (const IRExpr * expr, /*OUT*/ IROp * op) {
  switch (expr->tag) {
  case Iex_Unop:
    *op = expr->Iex.Unop.op;
    break;
  case Iex_Binop:
    *op = expr->Iex.Binop.op;
    break;
  case Iex_Triop:
    *op = expr->Iex.Triop.details->op;
    break;
  case Iex_Qop:
    *op = expr->Iex.Qop.details->op;
    break;
  default:
    return False;
  }
  return True;
}

/* Replace a given binary operation by a call to a function
 */
static void vr_replaceBinFpOpScal (IRSB* sb, IRStmt* stmt, IRExpr* expr,
				   const HChar* functionName, void* function,
				   Vr_Op op,
				   Vr_Prec prec,
				   Vr_Vec vec) {
  //instrumentation to count operation

  if(vr.verbose){
    IROp irop;
    if (vr_getOp (expr, &irop))
      vr_maybe_record_ErrorOp (VR_ERROR_SCALAR, irop);
  }

  if(!vr.instr_op[op] ) {
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  if(!vr.instr_scalar) {
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  op, prec,vec, True);

  //conversion before call
  IRExpr * arg1;
  IRExpr * arg2;

  arg1 = expr->Iex.Triop.details->arg2;
  //shift arg is not a bug :  IRRoundingMode(I32) x F64 x F64 -> F64 */
  arg2 = expr->Iex.Triop.details->arg3;

  IRType ity=Ity_I64; //type of call result
  if (prec==VR_PREC_FLT) {
    arg1=vr_F32toI64(sb,arg1);
    arg2=vr_F32toI64(sb,arg2);
    ity=Ity_I32;
  }
  if (prec==VR_PREC_DBL) {
    arg1=vr_F64toI64(sb,arg1);
    arg2=vr_F64toI64(sb,arg2);
  }

  //call
  IRTemp res= newIRTemp (sb->tyenv, ity);
  addStmtToIRSB (sb,
		 IRStmt_Dirty(unsafeIRDirty_1_N (res, 2,
						 functionName, VG_(fnptr_to_fnentry)(function),
						 mkIRExprVec_2 (arg1, arg2))));

  //conversion after call
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



static void vr_replaceBinFpOpLLO_slow_safe (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					    const HChar* functionName, void* function,
					    Vr_Op op,
					    Vr_Prec prec,
					    Vr_Vec vec){
  //instrumentation to count operation
  if(!vr.instr_op[op] ) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  op, prec,vec, True);
  //conversion before call
  IRExpr * arg1LL=NULL;
  IRExpr * arg1;
  IRExpr * arg2LL;
  IRType ity=Ity_I64;//type of call result

  arg1 = expr->Iex.Binop.arg1;
  arg2LL = expr->Iex.Binop.arg2;
  if (prec==VR_PREC_FLT) {
    arg1LL = vr_getLLFloat (sb, arg1);
    arg2LL = vr_getLLFloat (sb, arg2LL);
    arg1LL = vr_I32toI64 (sb, arg1LL);
    arg2LL = vr_I32toI64 (sb, arg2LL);
    ity=Ity_I32;
  }
  if (prec==VR_PREC_DBL) {
    arg1LL = vr_getLLDouble (sb, arg1);
    arg2LL = vr_getLLDouble (sb, arg2LL);
  }

  //call
  IRTemp res=newIRTemp (sb->tyenv, ity);
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1LL, arg2LL))));

  //update after call
  if (prec==VR_PREC_FLT){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Binop (Iop_SetV128lo32, arg1,IRExpr_RdTmp(res))));
  }
  if (prec==VR_PREC_DBL){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Binop (Iop_SetV128lo64,arg1,IRExpr_RdTmp(res))));
  }
}


static void vr_replaceBinFpOpLLO_fast_unsafe (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					      const HChar* functionName, void* function,
					      Vr_Op op,
					      Vr_Prec prec,
					      Vr_Vec vec){
  //instrumentation to count operation
  if(!vr.instr_op[op] ) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  op, prec,vec, True);
  //conversion before call

  IRExpr * arg1;
  IRExpr * arg2;
  
  IRType ity=Ity_I64;//type of call result

  arg1 = expr->Iex.Binop.arg1;
  arg2 = expr->Iex.Binop.arg2;
  if (prec==VR_PREC_FLT) {
    arg1 = vr_getLLFloat (sb, arg1);
    arg2 = vr_getLLFloat (sb, arg2);
    arg1 = vr_I32toI64 (sb, arg1);
    arg2 = vr_I32toI64 (sb, arg2);
    ity=Ity_I32;
  }
  if (prec==VR_PREC_DBL) {
    arg1 = vr_getLLDouble (sb, arg1);
    arg2 = vr_getLLDouble (sb, arg2);
  }

  //call
  IRTemp res=newIRTemp (sb->tyenv, ity);
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 2,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_2 (arg1, arg2))));

  //update after call
  IROp opReg;
  if (prec==VR_PREC_FLT) opReg = Iop_32UtoV128;
  if (prec==VR_PREC_DBL) opReg = Iop_64UtoV128;
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				   IRExpr_Unop (opReg, IRExpr_RdTmp(res))));
}


static void vr_replaceBinFpOpLLO(IRSB* sb, IRStmt* stmt, IRExpr* expr,
				 const HChar* functionName, void* function,
				 Vr_Op op,
				 Vr_Prec prec,
				 Vr_Vec vec){
  if(vr.unsafe_llo_optim){
    vr_replaceBinFpOpLLO_fast_unsafe(sb,stmt,expr,functionName,function,op,prec,vec);
  }else{
    vr_replaceBinFpOpLLO_slow_safe(sb,stmt,expr,functionName,function,op,prec,vec);
  }
}




static void vr_replaceBinFullSSE (IRSB* sb, IRStmt* stmt, IRExpr* expr,
				  const HChar* functionName, void* function,
				  Vr_Op op,
				  Vr_Prec prec,
				  Vr_Vec vec) {
  if(!vr.instr_op[op] ) {
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  op, prec,vec, True);

  if(!(
	 (vec==VR_VEC_FULL2 && prec==VR_PREC_DBL)
     ||
         (vec==VR_VEC_FULL4 && prec==VR_PREC_FLT)
       )){
    VG_(tool_panic) ( "vr_replaceBinFullSSE requires SSE instructions...  \n");
  }

  //conversion before call
  IRExpr * arg1 = expr->Iex.Triop.details->arg2;
  IRExpr * arg2 = expr->Iex.Triop.details->arg3;


  IRExpr *arg1Lo=vr_getLLDouble (sb, arg1);
  IRExpr *arg1Hi=vr_getHLDouble (sb, arg1);
  IRExpr *arg2Lo=vr_getLLDouble (sb, arg2);
  IRExpr *arg2Hi=vr_getHLDouble (sb, arg2);

  IRTemp res= newIRTemp (sb->tyenv, Ity_V128);



  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 3,
                                                 "", VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_5 (IRExpr_VECRET(),
								arg1Hi,arg1Lo,
								arg2Hi,arg2Lo))));
  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
}




static void vr_replaceBinFullAVX (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			       const HChar* functionName, void* function,
			       Vr_Op op,
			       Vr_Prec prec,
			       Vr_Vec vec) {
  if(!vr.instr_op[op] ) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  op, prec,vec,True);


  if(!(
	 (vec==VR_VEC_FULL4 && prec==VR_PREC_DBL)
     ||
         (vec==VR_VEC_FULL8 && prec==VR_PREC_FLT)
       )){
    VG_(tool_panic) ( "vr_replaceBinFullAVX requires AVX instructions...  \n");
  }

  //conversion before call
  IRExpr * arg1 = expr->Iex.Triop.details->arg2;
  IRExpr * arg2 = expr->Iex.Triop.details->arg3;


  IRExpr* arg1Tab[4];
  IRExpr* arg2Tab[4];
  vr_getTabArgAVX (sb, arg1, arg1Tab);
  vr_getTabArgAVX (sb, arg2, arg2Tab);
   
  IRTemp res= newIRTemp (sb->tyenv, Ity_V256);



  //call 
  
  /* 1 call avx
    addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_9 (IRExpr_VECRET(),
								arg1Tab[0],arg1Tab[1], arg1Tab[2],arg1Tab[3],
								arg2Tab[0],arg2Tab[1], arg2Tab[2],arg2Tab[3]
								)
								)));*/


  if( prec==VR_PREC_DBL){
    addStmtToIRSB (sb,
		   IRStmt_Dirty(unsafeIRDirty_0_N (1,
						   "vr_AvxDoubleCopyFirstArg", VG_(fnptr_to_fnentry)(&vr_AvxDoubleCopyFirstArg),
						   mkIRExprVec_4 (arg1Tab[0],arg1Tab[1], arg1Tab[2],arg1Tab[3])
						   )));
  }else if(prec==VR_PREC_FLT){
     addStmtToIRSB (sb,
		   IRStmt_Dirty(unsafeIRDirty_0_N (1,
						   "vr_AvxFloatCopyFirstArg", VG_(fnptr_to_fnentry)(&vr_AvxFloatCopyFirstArg),
						   mkIRExprVec_4 (arg1Tab[0],arg1Tab[1], arg1Tab[2],arg1Tab[3])
						   )));
  }
    
    
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_5 (IRExpr_VECRET(),
								arg2Tab[0],arg2Tab[1], arg2Tab[2],arg2Tab[3]
								)
						 )));


  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
}




static void vr_replaceFMA (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			   const HChar* functionName, void* function,
			   Vr_Op   Op,
			   Vr_Prec Prec) {
  if(!vr.instr_op[Op] ) {
    vr_countOp (sb,  Op, Prec, VR_VEC_LLO,False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  Op, Prec, VR_VEC_LLO,True);

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



static void vr_replaceCast (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			    const HChar* functionName, void* function,
			    Vr_Op   Op,
			    Vr_Prec Prec) {
  if(!vr.instr_op[Op] ) {
    vr_countOp (sb,  Op, Prec, VR_VEC_SCAL,False);
    addStmtToIRSB (sb, stmt);
    return;
  }
  vr_countOp (sb,  Op, Prec, VR_VEC_SCAL,True);

  IRExpr * arg2 = expr->Iex.Binop.arg2;

  IRTemp res = newIRTemp (sb->tyenv, Ity_I64);

  arg2=vr_F64toI64(sb,arg2);

  IRDirty* id= unsafeIRDirty_1_N (res, 1,
				  functionName, VG_(fnptr_to_fnentry)(function),
				  mkIRExprVec_1 (arg2));

  addStmtToIRSB (sb,IRStmt_Dirty(id));



  IRExpr* conv=vr_I64toI32(sb, IRExpr_RdTmp(res ));
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				   IRExpr_Unop (Iop_ReinterpI32asF32, conv )));
}





static void vr_instrumentOp (IRSB* sb, IRStmt* stmt, IRExpr * expr, IROp op) {
    switch (op) {

      // Addition

      // - Double precision
    case Iop_AddF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Add64F", vr_Add64F, VR_OP_ADD,VR_PREC_DBL,VR_VEC_SCAL);

    case Iop_Add64F0x2: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Add64F", vr_Add64F,VR_OP_ADD, VR_PREC_DBL,VR_VEC_LLO);

    case Iop_Add64Fx2: // 128b vector, 2 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Add64Fx2", vr_Add64Fx2,VR_OP_ADD, VR_PREC_DBL,VR_VEC_FULL2);

    case Iop_AddF32: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Add32F", vr_Add32F, VR_OP_ADD,VR_PREC_FLT,VR_VEC_SCAL);

    case Iop_Add32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Add32F", vr_Add32F, VR_OP_ADD,VR_PREC_FLT,VR_VEC_LLO);

    case Iop_Add32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Add32Fx4", vr_Add32Fx4,VR_OP_ADD, VR_PREC_FLT,VR_VEC_FULL4);

    case Iop_Add64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Add64Fx4", vr_Add64Fx4,VR_OP_ADD, VR_PREC_DBL,VR_VEC_FULL4);

    case Iop_Add32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Add32Fx8", vr_Add32Fx8,VR_OP_ADD, VR_PREC_FLT,VR_VEC_FULL8);


      // Subtraction

      // - Double precision
    case Iop_SubF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Sub64F", vr_Sub64F, VR_OP_SUB,VR_PREC_DBL,VR_VEC_SCAL);

    case Iop_Sub64F0x2: // 128b vector, lowest-lane only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Sub64F", vr_Sub64F, VR_OP_SUB, VR_PREC_DBL,VR_VEC_LLO);

    case Iop_Sub64Fx2:
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Sub64Fx2", vr_Sub64Fx2, VR_OP_SUB, VR_PREC_DBL,VR_VEC_FULL2);

    case Iop_SubF32: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Sub32F", vr_Sub32F, VR_OP_SUB,VR_PREC_FLT,VR_VEC_SCAL);
      
    case Iop_Sub32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Sub32F", vr_Sub32F, VR_OP_SUB,VR_PREC_FLT,VR_VEC_LLO);

    case Iop_Sub32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Sub32Fx4", vr_Sub32Fx4,VR_OP_SUB, VR_PREC_FLT,VR_VEC_FULL4);

    case Iop_Sub64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Sub64Fx4", vr_Sub64Fx4,VR_OP_SUB, VR_PREC_DBL,VR_VEC_FULL4);

    case Iop_Sub32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Sub32Fx8", vr_Sub32Fx8,VR_OP_SUB, VR_PREC_FLT,VR_VEC_FULL8);

      // Multiplication

      // - Double precision
    case Iop_MulF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Mul64F", vr_Mul64F, VR_OP_MUL,VR_PREC_DBL,VR_VEC_SCAL);

    case Iop_Mul64F0x2: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Mul64F", vr_Mul64F, VR_OP_MUL,VR_PREC_DBL,VR_VEC_LLO);

    case Iop_Mul64Fx2: // 128b vector, 2 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Mul64Fx2", vr_Mul64Fx2, VR_OP_MUL,VR_PREC_DBL,VR_VEC_FULL2);

    case Iop_MulF32: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Mul32F", vr_Mul32F,VR_OP_MUL, VR_PREC_FLT,VR_VEC_SCAL);

    case Iop_Mul32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Mul32F", vr_Mul32F, VR_OP_MUL,VR_PREC_FLT,VR_VEC_LLO);

    case Iop_Mul32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Mul32Fx4", vr_Mul32Fx4,VR_OP_MUL, VR_PREC_FLT,VR_VEC_FULL4);      

    case Iop_Mul64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Mul64Fx4", vr_Mul64Fx4,VR_OP_MUL, VR_PREC_DBL,VR_VEC_FULL4);

    case Iop_Mul32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Mul32Fx8", vr_Mul32Fx8,VR_OP_MUL, VR_PREC_FLT,VR_VEC_FULL8);

    case Iop_DivF32:
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Div32F", vr_Div32F, VR_OP_DIV,VR_PREC_FLT,VR_VEC_SCAL);

    case Iop_Div32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Div32F", vr_Div32F, VR_OP_DIV,VR_PREC_FLT,VR_VEC_LLO);

    case Iop_Div32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr,"vr_Div32Fx4", vr_Div32Fx4,VR_OP_DIV, VR_PREC_FLT,VR_VEC_FULL4);      

    case Iop_DivF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr,"vr_Div64F", vr_Div64F, VR_OP_DIV,VR_PREC_DBL,VR_VEC_SCAL);

    case Iop_Div64F0x2: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr,"vr_Div64F", vr_Div64F,VR_OP_DIV, VR_PREC_DBL,VR_VEC_LLO);

    case Iop_Div64Fx2: // 128b vector, 2 lanes
      return vr_replaceBinFullSSE(sb, stmt, expr,"vr_Div64Fx2", vr_Div64Fx2,VR_OP_DIV, VR_PREC_DBL,VR_VEC_FULL2);

    case Iop_Div64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Div64Fx4", vr_Div64Fx4,VR_OP_DIV, VR_PREC_DBL,VR_VEC_FULL4);
      
    case Iop_Div32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr,"vr_Div32Fx8", vr_Div32Fx8,VR_OP_DIV, VR_PREC_FLT,VR_VEC_FULL8);

      

    case Iop_MAddF32:
      return vr_replaceFMA (sb, stmt, expr,"vr_MAdd32F", vr_MAdd32F, VR_OP_MADD, VR_PREC_FLT);

    case Iop_MSubF32:
      return vr_replaceFMA (sb, stmt, expr,"vr_MSub32F", vr_MSub32F, VR_OP_MSUB, VR_PREC_FLT);

    case Iop_MAddF64:
      return vr_replaceFMA (sb, stmt, expr,"vr_MAdd64F", vr_MAdd64F, VR_OP_MADD, VR_PREC_DBL);

    case Iop_MSubF64:
      return vr_replaceFMA (sb, stmt, expr,"vr_MSub64F", vr_MSub64F,VR_OP_MSUB,  VR_PREC_DBL);


      //   Other FP operations
    case Iop_Add32Fx2:
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;


    case Iop_Sub32Fx2:
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
      
    case Iop_CmpF64:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_CmpF32:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F32toF64:  /*                       F32 -> F64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_FLT_TO_DBL, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toF32:
      return vr_replaceCast (sb, stmt, expr,"vr_Cast64FTo32F", vr_Cast64FTo32F,VR_OP_CONV,  VR_PREC_DBL_TO_FLT);
      break;

    case Iop_F64toI64S: /* IRRoundingMode(I32) x F64 -> signed I64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_INT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI64U: /* IRRoundingMode(I32) x F64 -> unsigned I64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_INT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI32S: /* IRRoundingMode(I32) x F64 -> signed I32 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_SHT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI32U: /* IRRoundingMode(I32) x F64 -> unsigned I32 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_SHT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

      /******/
    case Iop_Max32Fx4:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max32F0x4:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max64Fx2:
        vr_countOp (sb, VR_OP_MAX, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max64F0x2:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;


      

    case Iop_Min32Fx4:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min32F0x4:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min64Fx2:
        vr_countOp (sb, VR_OP_MIN, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min64F0x2:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;


    case Iop_CmpEQ64Fx2: case Iop_CmpLT64Fx2:
    case Iop_CmpLE64Fx2: case Iop_CmpUN64Fx2:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ64F0x2: case Iop_CmpLT64F0x2:
    case Iop_CmpLE64F0x2: case Iop_CmpUN64F0x2:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ32Fx4: case Iop_CmpLT32Fx4:
    case Iop_CmpLE32Fx4: case Iop_CmpUN32Fx4:
    case Iop_CmpGT32Fx4: case Iop_CmpGE32Fx4:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ32F0x4: case Iop_CmpLT32F0x4:
    case Iop_CmpLE32F0x4: case Iop_CmpUN32F0x4:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_ReinterpF64asI64:
    case Iop_ReinterpI64asF64:
    case Iop_ReinterpF32asI32:
    case Iop_ReinterpI32asF32:
    case Iop_NegF64:
    case Iop_AbsF64:
    case Iop_NegF32:
    case Iop_AbsF32:
    case Iop_Abs64Fx2:
    case Iop_Neg64Fx2:
      //ignored : not counted and not instrumented
      addStmtToIRSB (sb, stmt);
      break;

      //operation with 64bit register with 32bit rounding
    case Iop_AddF64r32:
    case Iop_SubF64r32:
    case Iop_MulF64r32:
    case Iop_DivF64r32:
    case Iop_MAddF64r32:
    case Iop_MSubF64r32:

      //operation with 128bit
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

    case Iop_RoundF64toF64_NEAREST: /* frin */
    case Iop_RoundF64toF64_NegINF:  /* frim */
    case Iop_RoundF64toF64_PosINF:  /* frip */
    case Iop_RoundF64toF64_ZERO:    /* friz */

    case Iop_F128toF64:  /* IRRoundingMode(I32) x F128 -> F64         */
    case Iop_F128toF32:  /* IRRoundingMode(I32) x F128 -> F32         */
    case Iop_F64toI16S: /* IRRoundingMode(I32) x F64 -> signed I16 */

    case Iop_CmpF128:

    case Iop_PwMax32Fx4: case Iop_PwMin32Fx4:
      vr_maybe_record_ErrorOp (VR_ERROR_UNCOUNTED, op);

    default:
      //      ppIRStmt (stmt);
      addStmtToIRSB (sb, stmt);
      break;
    }

}

static void vr_instrumentExpr (IRSB* sb, IRStmt* stmt, IRExpr* expr) {
  IROp op;
  //  ppIRStmt(stmt);VG_(printf)("\n");
  if (vr_getOp (expr, &op)) {
    vr_instrumentOp (sb, stmt, expr, op);
  } else {
    addStmtToIRSB (sb, stmt);
  }
}

// * Valgrind tool interface

static
IRSB* vr_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      const VexGuestLayout* layout,
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  const HChar * fnname;
  const HChar * objname;
  if (vr_excludeIRSB (&fnname, &objname))
    return sbIn;


  UInt i;
  IRSB* sbOut = deepCopyIRSBExceptStmts(sbIn);
  Bool includeSource = True;
  for (i=0 ; i<sbIn->stmts_used ; ++i) {
    IRStmt* st = sbIn->stmts[i];

    switch (st->tag) {
    case Ist_IMark: {
      //      HChar filename[256];
      const HChar *filename;
      UInt  linenum;
      Addr  addr;

      addr = st->Ist.IMark.addr;

      //      filename[0] = 0;
      VG_(get_filename_linenum)(VG_(current_DiEpoch)(),
                                addr,
                                &filename,
                                NULL,
                                &linenum);
      includeSource = vr_includeSource (&vr.includeSource, vr.genIncludeSource, fnname, filename, linenum);
    }
      break;
    case Ist_WrTmp:
      if (includeSource) {
        vr_instrumentExpr (sbOut, st, st->Ist.WrTmp.data);
        break;
      }
    default:
      addStmtToIRSB (sbOut, sbIn->stmts[i]);
    }
  }

  return sbOut;
}

static void vr_fini(Int exitcode)
{

  if (CHECK_C != 0) {
    VG_(fclose)(vr_outCancellationFile);
  }


  vr_ppOpCount ();
  interflop_verrou_finalyze(backend_context);

  if (vr.genExclude) {
    vr_dumpExcludeList(vr.exclude, vr.genExcludeUntil,
                       vr.excludeFile);
  }

  if (vr.genIncludeSource) {
    vr_dumpIncludeSourceList (vr.includeSource, vr.genIncludeSourceUntil,
                              vr.includeSourceFile);
  }

  vr_freeExcludeList (vr.exclude);
  vr_freeIncludeSourceList (vr.includeSource);
  VG_(free)(vr.excludeFile);
  VG_(free)(vr.genAbove);
}

void vr_cancellation_handler(int cancelled ){
  VG_(fprintf)(vr_outCancellationFile, "C  %d\n", cancelled);
}

static void print_op(int nbArg, const char* name, const double* args,const double* res){
  if(nbArg==1){
    VG_(umsg)("%s : %f => %f\n", name,args[0],*res);
  }
  if(nbArg==2){
    VG_(umsg)("%s : %f, %f => %f\n", name,args[0], args[1],*res);
  }
  if(nbArg==3){
    VG_(umsg)("%s : %f, %f, %f => %f\n", name, args[0], args[1], args[2], *res);
  }
}


static void vr_post_clo_init(void)
{
   // Values coming from the environment take precedence over CLOs
   vr_env_clo("VERROU_ROUNDING_MODE", "--rounding-mode");
   vr_env_clo("VERROU_INSTR_ATSTART", "--instr-atstart");
   vr_env_clo("VERROU_EXCLUDE",       "--exclude");
   vr_env_clo("VERROU_GEN_EXCLUDE",   "--gen-exclude");
   vr_env_clo("VERROU_GEN_ABOVE",     "--gen-above");
   vr_env_clo("VERROU_SOURCE",        "--source");
   vr_env_clo("VERROU_GEN_SOURCE",    "--gen-source");

   if (vr.genExclude) {
     vr.genExcludeUntil = vr.exclude;
   }

   if (vr.genAbove == NULL) {
     vr.genAbove = VG_(strdup)("vr.post_clo_init.gen-above", "main");
   }


   //Verrou Backend Initilisation
   backend=interflop_verrou_init(&backend_context);
   verrou_set_panic_handler(&VG_(tool_panic));
   verrou_set_nan_handler(&vr_handle_NaN);
   verrou_set_debug_print_op(&print_op);//Use only verrou backend is configured to use it

   VG_(umsg)("Backend %s : %s\n", interflop_verrou_get_backend_name(), interflop_verrou_get_backend_version()  );

   interflop_verrou_configure(vr.roundingMode,backend_context);
   
   //Random Seed initialisation
   if(( (vr.roundingMode== VR_RANDOM) || (vr.roundingMode== VR_AVERAGE))){
     if(vr.firstSeed==(unsigned int )(-1)){
       struct vki_timeval now;
       VG_(gettimeofday)(&now, NULL);
       unsigned int pid = VG_(getpid)();
       vr.firstSeed = now.tv_usec + pid;
     }
     VG_(umsg)("First seed : %u\n", vr.firstSeed);
     verrou_set_seed (vr.firstSeed);
   }
   VG_(umsg)("Simulating %s rounding mode\n", verrou_rounding_mode_name (vr.roundingMode));
   
   /*Init outfile cancellation*/
   if (CHECK_C != 0) {
     vr_outCancellationFile = VG_(fopen)("vr.log",
					 VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
					 VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
     verrou_set_cancellation_handler(&vr_cancellation_handler);
   }


   /*If no operation selected the default is all*/
   Bool someThingInstr=False;
   int opIt;
   for(opIt=0; opIt< VR_OP ;opIt++){
     if(vr.instr_op[opIt]) someThingInstr=True;
   }
   if(!someThingInstr){
     for(opIt=0; opIt<  VR_OP_CMP ;opIt++){ // Instruction after VR_OP_CMP (included) are not instrumented
       vr.instr_op[opIt]=True;
     }
     vr.instr_op[VR_OP_CONV]=True;
   }
   VG_(umsg)("Instrumented operations :\n");
   for (opIt=0; opIt< VR_OP ;opIt++){
     VG_(umsg)("\t%s : ", vr_ppOp(opIt));
     if(vr.instr_op[opIt]==True) VG_(umsg)("yes\n");
     else VG_(umsg)("no\n");
   }
   VG_(umsg)("Instrumented scalar operations : ");
   if(vr.instr_scalar) VG_(umsg)("yes\n");
   else VG_(umsg)("no\n");

   if(!vr.instrument){
     vr.instrument = True;
     vr_set_instrument_state ("Program start", False);
   }
}

static void vr_pre_clo_init(void)
{
   VG_(details_name)            ("Verrou");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("Check floating-point rounding errors");
   VG_(details_copyright_author)(
      "Copyright (C) 2014-2016, F. Fevotte & B. Lathuiliere.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );


   VG_(clo_vex_control).iropt_register_updates_default
     = VG_(clo_px_file_backed)
     = VexRegUpdSpAtMemAccess; // overridable by the user.

   VG_(clo_vex_control).iropt_unroll_thresh = 0;   // cannot be overriden.
   VG_(clo_vex_control).guest_chase_thresh = 0;    // cannot be overriden.

   VG_(basic_tool_funcs)        (vr_post_clo_init,
                                 vr_instrument,
                                 vr_fini);

   VG_(needs_command_line_options)(vr_process_clo,
                                   vr_print_usage,
                                   vr_print_debug_usage);

   VG_(needs_client_requests)(vr_handle_client_request);

   VG_(needs_tool_errors)(vr_eq_Error,
                          vr_before_pp_Error,
                          vr_pp_Error,
			  False,                          //show_ThreadIDs_for_errors
                          vr_update_extra,
                          vr_recognised_suppression,
                          vr_read_extra_suppression_info,
                          vr_error_matches_suppression,
                          vr_get_error_name,
                          vr_print_extra_suppression_info,
                          vr_print_extra_suppression_use,
                          vr_update_extra_suppression_use);

   vr_clo_defaults();
}

VG_DETERMINE_INTERFACE_VERSION(vr_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
