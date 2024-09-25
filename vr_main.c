/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                vr_main.c ---*/
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

#include "vr_main.h"
#include "float.h"
#include "pub_tool_seqmatch.h"


//#include "coregrind/pub_core_transtab.h"
#include "pub_tool_transtab.h"
extern Bool  VG_(ok_to_discard_translations);

//#include "coregrind/pub_core_debuginfo.h"
#include "pub_tool_debuginfo.h"
extern Bool VG_(get_fnname_raw) ( DiEpoch ep, Addr a, const HChar** buf );

//#include "coregrind/pub_core_libcfile.h"
#include "pub_tool_libcfile.h"
extern Bool VG_(resolve_filename) ( Int fd, const HChar** buf );



Vr_State vr;

//
#if defined(VG_BIGENDIAN)
# define ENDIANNESS Iend_BE
#elif defined(VG_LITTLEENDIAN)
# define ENDIANNESS Iend_LE
#else
# error "Unknown endianness"
#endif

typedef struct {
  Bool containFloatModOp;
  Bool containFloatCmp;
} Vr_instr_kind;


struct interflop_backend_interface_t backend_verrou;
void* backend_verrou_context;

struct interflop_backend_interface_t backend_verrou_null;//used for soft stop/start
void* backend_verrou_null_context;

struct interflop_backend_interface_t backend_mcaquad;
void* backend_mcaquad_context;

struct interflop_backend_interface_t backend_checkcancellation;
void* backend_checkcancellation_context;

struct interflop_backend_interface_t backend_checkdenorm;
void* backend_checkdenorm_context;

struct interflop_backend_interface_t backend_check_float_max;
void* backend_check_float_max_context;





VgFile * vr_outCancellationFile;
VgFile * vr_outDenormFile;

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
  case VR_OP_SQRT:
    return "sqrt";
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


static const HChar* vr_ppPrec (Vr_Prec prec) {
  switch (prec) {
  case VR_PREC_FLT:
    return "flt";
  case VR_PREC_DBL:
    return "dbl";
  case VR_PREC_LDBL:
    return "ldbl";
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
  case VR_VEC_UNK:
    return "unk ";

  default:
    return "unknown";
  }
}

// ** Counter handling


static ULong vr_opCount[VR_OP][VR_PREC][VR_VEC][VR_INSTR];

void vr_resetCount(void){
   Vr_Op op;
   Vr_Prec prec;
   Vr_Vec vec;
   for (op = 0 ; op<VR_OP ; ++op) {
      for (prec = 0 ; prec < VR_PREC ; ++prec) {
         for (vec = 0 ; vec < VR_VEC ; ++vec) {
           vr_opCount[op][prec][vec][VR_INSTR_ON]=0;
           vr_opCount[op][prec][vec][VR_INSTR_OFF]=0;
         }
      }
   }
};

static VG_REGPARM(2) void vr_incOpCount (ULong* counter, SizeT increment) {
   counter[(vr.instrument_hard && vr.instrument_soft)] += increment;
}

static VG_REGPARM(2) void vr_incUninstrumentedOpCount (ULong* counter, SizeT increment) {
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
			    "vr_incUninstrumentedOpCount",
			    VG_(fnptr_to_fnentry)( &vr_incUninstrumentedOpCount ),
			    argv);

  }

  addStmtToIRSB (sb, IRStmt_Dirty (di));
}



static unsigned int vr_frac (ULong a, ULong b) {
  unsigned int q = (100*a)/(a+b);
  if (100*a - (a+b)*q > (a+b)/2) {q++;}
  return q;
}

UInt vr_count_fp_instrumented(void){
   //Warning : the return type is small (to be consistent with return client request type)
   //should be used only  as heuristic  to detect dynamicaly fp operations
   if(!vr.count) {
      VG_(tool_panic) ( "--count-op=no not allowed with vr_count_fp_instrumented function \n");
   }
   Vr_Op op;
   Vr_Prec prec;
   Vr_Vec vec;
   ULong total=0;
   for (op = 0 ; op<VR_OP ; ++op) {
      for (prec = 0 ; prec < VR_PREC ; ++prec) {
         for (vec = 0 ; vec < VR_VEC ; ++vec) {
            total  += vr_opCount[op][prec][vec][VR_INSTR_ON];
         }
      }
   }
   return (UInt)total;
}


UInt vr_count_fp_not_instrumented(void){
   //Warning : the return type is small (to be consistent with return client request type)
   //should be used only  as heuristic  to detect dynamicaly fp operations
   if(!vr.count) {
      VG_(tool_panic) ( "--count-op=no not allowed with vr_count_fp_not_instrumented function \n");
   }
   Vr_Op op;
   Vr_Prec prec;
   Vr_Vec vec;
   ULong total=0;
   for (op = 0 ; op<VR_OP ; ++op) {
      for (prec = 0 ; prec < VR_PREC ; ++prec) {
         for (vec = 0 ; vec < VR_VEC ; ++vec) {
            total  += vr_opCount[op][prec][vec][VR_INSTR_OFF];
         }
      }
   }
   return (UInt)total;
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
	    if(vec==VR_VEC_UNK){
	      continue;
	    }
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

static VG_REGPARM(0) void vr_updatep_prandom (void) {
  verrou_updatep_prandom();
}

void vr_generate_exclude_source(const char* functionName, int line, const char* fileName, const char* object ){
   if(vr.genExcludeBool){
    vr_excludeIRSB_generate (&functionName, &object);
   }
   if(vr.genIncludeSource){
     vr_includeSource_generate (&vr.gen_includeSource, functionName, fileName, line);
   }
}

Bool vr_clrIsInstrumented(const char* functionName, int line, const char* fileName, const char* object){
  if(vr.verbose){
    VG_(umsg)("function Name: %s\tline: %d\tfileName: %s\tobject: %s\n",functionName, line, fileName, object);
  }
  Bool excludeIrsb=vr_excludeIRSB (&functionName, &object, NULL);
   if(excludeIrsb){
      return False;
   }

   if(vr.sourceActivated){
     Bool sourceInclude= vr_includeSource(&vr.includeSource, functionName, fileName, line);
     return sourceInclude;
   }else{
     return True;
   }
}

unsigned int* cacheTab[10];
unsigned int  cacheSizeTab[10];
unsigned int nbRegistredCache=0;
void vr_register_cache(unsigned int* cache, unsigned int size){
   if(nbRegistredCache==10){
      VG_(tool_panic)("too much cache");
   }
   cacheTab[nbRegistredCache]=cache;
   cacheSizeTab[nbRegistredCache]=size;
   nbRegistredCache++;
}

void vr_clean_cache(void){
   for(unsigned int i=0; i< nbRegistredCache;i++){
      VG_(memset)(cacheTab[i],0,cacheSizeTab[i]) ;
   }
}

unsigned int* cacheSeed=NULL;
void vr_register_cache_seed(unsigned int* cache){
  cacheSeed=cache;
}

void vr_clean_cache_seed(){
  if(cacheSeed!=NULL){
    *cacheSeed=0;
  }
}


#include "vr_traceBB_impl.h"


// * Floating point operations overload


// ** Overloaded operators


#include "vr_interp_operator_impl.h"
#include "vr_generated_from_templates.h"


// *** Helpers

/* Return the Lowest Lane of a given packed temporary register */

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


static Bool vr_isInstrumented_hard(Vr_Op op,
			      Vr_Prec prec,
			      Vr_Vec vec){
  return vr.instr_op[op] && vr.instr_vec[vec]&&vr.instr_prec[prec] && vr.instrument_hard;
}

/* Replace a given binary operation by a call to a function
 */
static Bool vr_replaceBinFpOpScal (IRSB* sb, IRStmt* stmt, IRExpr* expr,
				   const HChar* functionName, void* function,
				   Vr_Op op,
				   Vr_Prec prec,
				   Vr_Vec vec,
				   Bool countOnly) {
  // un-instrumented cases :
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
     vr_countOp (sb,  op, prec,vec, False);
     addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  // instrumented case :
  if(vr.verbose){
    IROp irop;
    if (vr_getOp (expr, &irop))
      vr_maybe_record_ErrorOp (VR_ERROR_SCALAR, irop);
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
#if defined(VGA_amd64)
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
#elif defined(VGA_arm64)
//arm64
  if(prec==VR_PREC_FLT){
     addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                      IRExpr_Unop (Iop_ReinterpI32asF32, IRExpr_RdTmp(res ) )));
  }
  if(prec==VR_PREC_DBL){
     addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                      IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
  }
#else
#error "not yet implemented"
#endif

  return True;
}


static Bool vr_replaceBinFpOpScal_unary (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					 const HChar* functionName, void* function,
					 Vr_Op op,
					 Vr_Prec prec,
					 Vr_Vec vec,
					 Bool countOnly) {
  // un-instrumented cases :
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
     vr_countOp (sb,  op, prec,vec, False);
     addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  // instrumented case :
  if(vr.verbose){
    IROp irop;
    if (vr_getOp (expr, &irop))
      vr_maybe_record_ErrorOp (VR_ERROR_SCALAR, irop);
  }

  vr_countOp (sb,  op, prec,vec, True);

  //conversion before call
  IRExpr * arg1;

  arg1 = expr->Iex.Binop.arg2;
  //shift arg is not a bug :  IRRoundingMode(I32) x F64  -> F64 */

  IRType ity=Ity_I64; //type of call result
  if (prec==VR_PREC_FLT) {
    arg1=vr_F32toI64(sb,arg1);
    ity=Ity_I32;
  }
  if (prec==VR_PREC_DBL) {
    arg1=vr_F64toI64(sb,arg1);
  }

  //call
  IRTemp res= newIRTemp (sb->tyenv, ity);
  addStmtToIRSB (sb,
		 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
						 functionName, VG_(fnptr_to_fnentry)(function),
						 mkIRExprVec_1 (arg1))));

  //conversion after call
#if defined(VGA_amd64)
  if(prec==VR_PREC_FLT){
      IRExpr* conv=vr_I64toI32(sb, IRExpr_RdTmp(res ));
      addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI32asF32, conv )));
  }
  if(prec==VR_PREC_DBL){
      addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				       IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
  }
#elif defined(VGA_arm64)
  if(prec==VR_PREC_FLT){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI32asF32, IRExpr_RdTmp(res ))));
  }
  if(prec==VR_PREC_DBL){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
  }
#else
#error "not yet implemented"
#endif
  return True;
}



static Bool vr_replaceBinFpOpLLO_slow_safe (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					    const HChar* functionName, void* function,
					    Vr_Op op,
					    Vr_Prec prec,
					    Vr_Vec vec,
					    Bool countOnly){
  //instrumentation to count operation
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  vr_countOp (sb,  op, prec,vec, True);

  IRExpr * arg1 = expr->Iex.Binop.arg1;
  IRExpr * arg2 = expr->Iex.Binop.arg2;

  if (prec==VR_PREC_FLT) {
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyFloat), arg2));
  }
  if (prec==VR_PREC_DBL) {
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyDouble), arg2));
  }

  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_0_N( 0,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
						 mkIRExprVec_0())));
  //update after call
  if (prec==VR_PREC_FLT){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                     IRExpr_Load(ENDIANNESS, Ity_V128, mkIRExpr_HWord ((HWord) arg1CopyFloat))));
  }
  if (prec==VR_PREC_DBL){
     addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                      IRExpr_Load(ENDIANNESS, Ity_V128, mkIRExpr_HWord ((HWord) arg1CopyDouble))));
  }
  return True;
}



static Bool vr_replaceBinFpOpLLO_unary_slow_safe (IRSB* sb, IRStmt* stmt, IRExpr* expr,
						  const HChar* functionName, void* function,
						  Vr_Op op,
						  Vr_Prec prec,
						  Vr_Vec vec,
						  Bool countOnly){
  //instrumentation to count operation
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  vr_countOp (sb,  op, prec,vec, True);

  IRExpr * arg1 = expr->Iex.Binop.arg1;

  if (prec==VR_PREC_FLT) {
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
  }
  if (prec==VR_PREC_DBL) {
    addStmtToIRSB(sb, IRStmt_Store   ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
  }

  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_0_N ( 0,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_0 ())));
  //update after call
  if (prec==VR_PREC_FLT){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                     IRExpr_Load(ENDIANNESS, Ity_V128, mkIRExpr_HWord ((HWord) arg1CopyFloat))));
  }
  if (prec==VR_PREC_DBL){
     addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
                                      IRExpr_Load(ENDIANNESS, Ity_V128, mkIRExpr_HWord ((HWord) arg1CopyDouble))));
  }
  return True;
}

static Bool vr_replaceBinFpOpLLO_unary (IRSB* sb, IRStmt* stmt, IRExpr* expr,
						  const HChar* functionName, void* function,
						  Vr_Op op,
						  Vr_Prec prec,
						  Vr_Vec vec,
						  Bool countOnly){
  return vr_replaceBinFpOpLLO_unary_slow_safe (sb, stmt, expr,
					       functionName, function,
					       op, prec, vec,
					       countOnly);
}

static Bool vr_replaceBinFpOpLLO(IRSB* sb, IRStmt* stmt, IRExpr* expr,
				 const HChar* functionName, void* function,
				 Vr_Op op,
				 Vr_Prec prec,
				 Vr_Vec vec,
				 Bool countOnly){
   return vr_replaceBinFpOpLLO_slow_safe(sb,stmt,expr,functionName,function,op,prec,vec,countOnly);
}




static Bool vr_replaceBinFullSSE (IRSB* sb, IRStmt* stmt, IRExpr* expr,
				  const HChar* functionName, void* function,
				  Vr_Op op,
				  Vr_Prec prec,
				  Vr_Vec vec,
				  Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return True;
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

  IRTemp res= newIRTemp (sb->tyenv, Ity_V128);

  if(prec==VR_PREC_DBL){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyDouble), arg2));
  }
  if(prec==VR_PREC_FLT){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyFloat), arg2));
  }
  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_1 (IRExpr_VECRET()))));
  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
  return True;
}

static Bool vr_replaceBinFullSSE_unary (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					const HChar* functionName, void* function,
					Vr_Op op,
					Vr_Prec prec,
					Vr_Vec vec,
					Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec, False);
    addStmtToIRSB (sb, stmt);
    return True;
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
  IRExpr * arg1 = expr->Iex.Binop.arg2;

  IRTemp res= newIRTemp (sb->tyenv, Ity_V128);

  if(prec==VR_PREC_DBL){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
  }
  if(prec==VR_PREC_FLT){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
  }

  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_1 (IRExpr_VECRET()))));
  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
  return True;
}



static Bool vr_replaceBinFullAVX (IRSB* sb, IRStmt* stmt, IRExpr* expr,
				  const HChar* functionName, void* function,
				  Vr_Op op,
				  Vr_Prec prec,
				  Vr_Vec vec,
				  Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return True;
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

  IRTemp res= newIRTemp (sb->tyenv, Ity_V256);
  if( prec==VR_PREC_DBL){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyDouble), arg2));
  }else if(prec==VR_PREC_FLT){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg2CopyFloat), arg2));
  }
  //call
  addStmtToIRSB (sb,
                 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
                                                 functionName, VG_(fnptr_to_fnentry)(function),
                                                 mkIRExprVec_1 (IRExpr_VECRET()))));
  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
  return True;
}


static Bool vr_replaceBinFullAVX_unary (IRSB* sb, IRStmt* stmt, IRExpr* expr,
					const HChar* functionName, void* function,
					Vr_Op op,
					Vr_Prec prec,
					Vr_Vec vec,
					Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,vec))) {
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec,vec,False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  vr_countOp (sb,  op, prec,vec,True);


  if(!(
	 (vec==VR_VEC_FULL4 && prec==VR_PREC_DBL)
     ||
         (vec==VR_VEC_FULL8 && prec==VR_PREC_FLT)
       )){
    VG_(tool_panic) ( "vr_replaceBinFullAVX_unary requires AVX instructions...  \n");
  }

  //conversion before call
  IRExpr * arg1 = expr->Iex.Binop.arg1;

  IRTemp res= newIRTemp (sb->tyenv, Ity_V256);
  if( prec==VR_PREC_DBL){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyDouble), arg1));
  }else if(prec==VR_PREC_FLT){
     addStmtToIRSB(sb, IRStmt_Store ( ENDIANNESS, mkIRExpr_HWord ((HWord) arg1CopyFloat), arg1));
  }

  addStmtToIRSB (sb,
		 IRStmt_Dirty(unsafeIRDirty_1_N (res, 1,
						 functionName, VG_(fnptr_to_fnentry)(function),
						 mkIRExprVec_1 (IRExpr_VECRET()))));

  //conversion after call
  addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp, IRExpr_RdTmp(res)));
  return True;
}



static Bool vr_replaceFMA (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			   const HChar* functionName, void* function,
			   Vr_Op   op,
			   Vr_Prec prec,
			   Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,VR_VEC_UNK))) {
    vr_countOp (sb,  op, prec, VR_VEC_UNK,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec, VR_VEC_UNK,False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  vr_countOp (sb,  op, prec, VR_VEC_UNK,True);

#ifdef USE_VERROU_FMA
  //  IRExpr * arg1 = expr->Iex.Qop.details->arg1; Rounding mode
  IRExpr * arg2 = expr->Iex.Qop.details->arg2;
  IRExpr * arg3 = expr->Iex.Qop.details->arg3;
  IRExpr * arg4 = expr->Iex.Qop.details->arg4;
  IRTemp res = newIRTemp (sb->tyenv, Ity_I64);
  if(prec== VR_PREC_DBL){
    arg2=vr_F64toI64(sb,arg2);
    arg3=vr_F64toI64(sb,arg3);
    arg4=vr_F64toI64(sb,arg4);

  }
  if(prec==VR_PREC_FLT){
    arg2=vr_F32toI64(sb,arg2);
    arg3=vr_F32toI64(sb,arg3);
    arg4=vr_F32toI64(sb,arg4);
  }

  IRDirty* id= unsafeIRDirty_1_N (res, 3,
				  functionName, VG_(fnptr_to_fnentry)(function),
				  mkIRExprVec_3 (arg2, arg3,arg4));

  addStmtToIRSB (sb,IRStmt_Dirty(id));



  if(prec==VR_PREC_FLT){
    IRExpr* conv=vr_I64toI32(sb, IRExpr_RdTmp(res ));
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI32asF32, conv )));
  }
  if(prec==VR_PREC_DBL){
    addStmtToIRSB (sb, IRStmt_WrTmp (stmt->Ist.WrTmp.tmp,
				     IRExpr_Unop (Iop_ReinterpI64asF64, IRExpr_RdTmp(res))));
  }
  return True;
#else //USE_VERROU_FMA
  //should not happed
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif //USE_VERROU_FMA
}




static Bool vr_replaceCast (IRSB* sb, IRStmt* stmt, IRExpr* expr,
			    const HChar* functionName, void* function,
			    Vr_Op   op,
			    Vr_Prec prec,
			    Bool countOnly) {
  if(!(vr_isInstrumented_hard(op,prec,VR_VEC_UNK))) {
    vr_countOp (sb,  op, prec, VR_VEC_UNK,False);
    addStmtToIRSB (sb, stmt);
    return False;
  }
  if(countOnly){
    vr_countOp (sb,  op, prec, VR_VEC_UNK, False);
    addStmtToIRSB (sb, stmt);
    return True;
  }

  vr_countOp (sb,  op, prec, VR_VEC_UNK,True);

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
  return True;
}





static Vr_instr_kind vr_instrumentOp (IRSB* sb, IRStmt* stmt, IRExpr * expr, IROp op, vr_backend_name_t bc, Bool countOnly) {
   Bool checkCancellation= (vr.checkCancellation || vr.dumpCancellation);
#ifndef USE_VERROU_SQRT
#define IGNORESQRT
#endif

   if(vr.backend==vr_verrou && !checkCancellation && ! vr.checkFloatMax){
#include "vr_instrumentOp_impl_generated_verrou_specific.h"
   }
#include "vr_instrumentOp_impl_generated_all_backends.h"

  Vr_instr_kind res;
  res.containFloatCmp=False;
  res.containFloatModOp=False;
  return res;
}

static Vr_instr_kind vr_instrumentExpr (IRSB* sb, IRStmt* stmt, IRExpr* expr, Bool countOnly) {
  IROp op;
  //  ppIRStmt(stmt);VG_(printf)("\n");
  if (vr_getOp (expr, &op)) {
    return vr_instrumentOp (sb, stmt, expr, op, vr.backend, countOnly);
  } else {
    addStmtToIRSB (sb, stmt);
    Vr_instr_kind res;
    res.containFloatModOp=False;
    res.containFloatCmp=False;
    return res;
  }
}

// * Valgrind tool interface
#define UNAMED_FUNCTION_VERROU "unamed_function_verrou"
#define UNAMED_OBJECT_VERROU "unamed_object_verrou"
#define UNAMED_FILENAME_VERROU "unamed_filename_verrou"




static HChar const * fnnoname=UNAMED_FUNCTION_VERROU;
static HChar const * objnoname=UNAMED_OBJECT_VERROU;
static HChar const * filenamenoname=UNAMED_FILENAME_VERROU;

inline
void vr_treat_line_from_imark(Bool excludeIrsb, Bool includeSource, Bool doLineContainFloat,   HChar const **  fnnamePtr,   const HChar ** filenamePtr, const UInt *linenumPtr){
  if( !excludeIrsb && doLineContainFloat){
    if(vr.genIncludeSource){
      vr_includeSource_generate (&vr.gen_includeSource, *fnnamePtr, *filenamePtr, *linenumPtr);
    }

    if(!includeSource&&  vr.sourceActivated && vr.sourceExcludeActivated&&!vr_includeSource(&vr.excludeSourceRead, *fnnamePtr, *filenamePtr, *linenumPtr)){
      VG_(umsg)("Warning new source line with fp operation discovered :\n");
      VG_(umsg)("\t%s : %s : %u\n", *fnnamePtr, *filenamePtr, *linenumPtr);
      vr.excludeSourceRead = vr_addIncludeSource (vr.excludeSourceRead,*fnnamePtr,*filenamePtr,*linenumPtr);//to print only once
    }
  }
}

static
IRSB* vr_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      const VexGuestLayout* layout,
                      const VexGuestExtents* vge,
                      const VexArchInfo* archinfo_host,
                      IRType gWordTy, IRType hWordTy )
{
  /*Recuperation of the name of symbol and object of the IRSB*/
  /*No externalized in a function because the call to get_fnname forbid other call 
    in the function vr_instrument : to use fnname after the end of call you have to use
    strdup
  */

  HChar const * fnname;
  HChar const * objname;
  HChar const ** fnnamePtr=&fnname;
  HChar const ** objnamePtr=&objname;

  Addr ips[256];
  VG_(get_StackTrace)(VG_(get_running_tid)(),ips, 256,
		      NULL, NULL,0);
  Addr addr = ips[0];
  DiEpoch de = VG_(current_DiEpoch)();

  Bool errorFnname=VG_(get_fnname_raw)(de, addr, fnnamePtr);
  if(!errorFnname || **fnnamePtr==0){
    fnnamePtr=&fnnoname;
  }

  if (VG_(strlen)(*fnnamePtr) == VR_FNNAME_BUFSIZE-1) {
    VG_(umsg)("WARNING: Function name too long: %s\n", *fnnamePtr);
  }

  Bool errorObjName=VG_(get_objname)(de, addr, objnamePtr);
  if (!errorObjName || **objnamePtr == 0) {
    objnamePtr=&objnoname;
  }
  /*End of recuperation*/

  Bool countIrsb=True;
  Bool excludeIrsb=vr_excludeIRSB (fnnamePtr, objnamePtr,&countIrsb);
  //if (excludeIrsb && !genIRSBTrace){
  //    return sbIn;
  //  }

  /*Instrumentation begin*/
  UInt i;
  IRSB* sbOut = deepCopyIRSBExceptStmts(sbIn);

  Vr_instr_kind instrStatus;

  Bool doIRSBFContainFloat=False;


  /*Data for Imark localisation*/
  Bool includeSource = True;
  Bool doLineContainFloat=False;
  Bool doLineContainFloatCmp=False;

  const HChar * filename=NULL;
  const HChar ** filenamePtr=&filenamenoname;
  UInt  linenum=0;
  UInt*  linenumPtr=&linenum;

  /*Data for trace/coverage generation*/
  traceBB_t* traceBB=NULL;
  Bool genIRSBTrace=vr.genTrace &&  vr_includeTraceIRSB(&fnname,&objname);
  if(genIRSBTrace){
    traceBB=getNewTraceBB(sbIn);
    vr_traceIRSB(sbOut,traceBB->index, &(traceBB->counter));//, instrCount);
    //vr_traceBB_trace_backtrace(traceBB);
  }


  /*Loop over instructions*/
  for (i=0 ; i<sbIn->stmts_used ; ++i) {
    IRStmt* st = sbIn->stmts[i];

    switch (st->tag) {
    case Ist_IMark: {
      vr_treat_line_from_imark(excludeIrsb,includeSource,doLineContainFloat,
			       fnnamePtr,filenamePtr,linenumPtr);
      if(genIRSBTrace){
	vr_traceBB_trace_imark(traceBB,
			       *fnnamePtr, *filenamePtr,*linenumPtr,
			       doLineContainFloat, doLineContainFloatCmp);
      }

      doLineContainFloat=False;
      doLineContainFloatCmp=False;
      Addr  addrMark;
      addrMark = st->Ist.IMark.addr;
      //      filename[0] = 0;
      filenamePtr=&filename;
      Bool success=VG_(get_filename_linenum)(VG_(current_DiEpoch)(),
					     addrMark,
					     filenamePtr,
					     NULL,
					     linenumPtr);
      if(! success || (**filenamePtr)==0){
	filenamePtr=&filenamenoname;
	*linenumPtr=0;
      }

      if(!vr.genIncludeSource){
	includeSource =(!vr.sourceActivated) || (vr.sourceActivated&&  vr_includeSource (&vr.includeSource, *fnnamePtr, *filenamePtr, *linenumPtr));
      }

      if(vr.prandomUpdate==VR_PRANDOM_UPDATE_FUNC){
	if(vr.roundingMode==VR_PRANDOM || vr.roundingMode==VR_PRANDOM_DET || vr.roundingMode==VR_PRANDOM_COMDET){
	  const HChar *localFnname;
	  if (VG_(get_fnname_if_entry)(de, st->Ist.IMark.addr, &localFnname)) {
	    IRDirty*   di;
	    di = unsafeIRDirty_0_N(0,
				   "vr_updatep_prandom", VG_(fnptr_to_fnentry)( &vr_updatep_prandom ),
				   mkIRExprVec_0() );
	    addStmtToIRSB( sbOut, IRStmt_Dirty(di) );
	    if(vr.verbose){
	     VG_(umsg)("prandom update instrumentation: %s\n", localFnname );
	    }
	  }
	}
      }

      addStmtToIRSB (sbOut, sbIn->stmts[i]); //required to be able to use breakpoint with gdb
    }
      break;
    case Ist_WrTmp:
      {
	if(countIrsb==False){
	  addStmtToIRSB (sbOut, sbIn->stmts[i]);
	  break;
	}

	Bool countOnly=False;
	if (excludeIrsb || !includeSource){
	  countOnly=True;
	}
	instrStatus=vr_instrumentExpr (sbOut, st, st->Ist.WrTmp.data,countOnly);
	Bool doInstrContainFloat=instrStatus.containFloatModOp;
	Bool doInstrContainFloatCmp=instrStatus.containFloatCmp;
	doLineContainFloat=doLineContainFloat   || doInstrContainFloat;
	doLineContainFloatCmp=doLineContainFloatCmp   || doInstrContainFloatCmp;
	doIRSBFContainFloat=doIRSBFContainFloat || doInstrContainFloat;

      }
      break;
    default:
      addStmtToIRSB (sbOut, sbIn->stmts[i]);
    }
  }

  vr_treat_line_from_imark(excludeIrsb,includeSource,doLineContainFloat,
			   fnnamePtr,filenamePtr,linenumPtr);
  if(genIRSBTrace){
    vr_traceBB_trace_imark(traceBB,
			   *fnnamePtr, *filenamePtr,*linenumPtr,
			   doLineContainFloat, doLineContainFloatCmp);
  }

  if(vr.genExcludeBool && !excludeIrsb &&doIRSBFContainFloat){
    vr_excludeIRSB_generate (fnnamePtr, objnamePtr);
  }
  return sbOut;
}

static void vr_fini(Int exitcode)
{

  //if (vr.checkCancellation) {
     //VG_(fclose)(vr_outCancellationFile);
  //}
#ifdef PROFILING_EXACT
  unsigned int num;
  unsigned int numExact;
  verrou_get_profiling_exact(&num,&numExact);
  VG_(umsg)("%s #total: %u #exact: %u\n","Profiling exact - last -", num, numExact);
#endif

  if(vr.useIOMatchCLR){
      vr_IOmatch_clr_finalize();
   }

  vr_ppOpCount ();
  interflop_verrou_finalize(backend_verrou_context);
  interflop_verrou_finalize(backend_verrou_null_context);
#ifdef USE_VERROU_QUADMATH
  interflop_mcaquad_finalize(backend_mcaquad_context);
#endif
  interflop_checkcancellation_finalize(backend_checkcancellation_context);
  interflop_check_float_max_finalize(backend_check_float_max_context);


  if (vr.genExcludeBool) {
    vr_dumpExcludeList(vr.gen_exclude, vr.excludeFile);
  }

  if (vr.genIncludeSource) {
    vr_dumpIncludeSourceList (vr.gen_includeSource, vr.includeSourceFile);
  }

  if(vr.genTrace){
    vr_traceBB_dumpCov();
    vr_traceBB_finalize();
  }
  if (vr.dumpCancellation){
     vr_dumpIncludeSourceList(vr.cancellationSource, vr.cancellationDumpFile );
  }

  if (vr.dumpDenorm){
     vr_dumpIncludeSourceList(vr.denormSource, vr.denormDumpFile );
  }
  vr_freeExcludeList (vr.exclude);
  vr_freeExcludeList (vr.gen_exclude);

  vr_freeIncludeSourceList (vr.includeSource);
  vr_freeIncludeSourceList( vr.excludeSourceRead);
  vr_freeIncludeTraceList  (vr.includeTrace );
  VG_(free)(vr.excludeFile);
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
   vr_env_clo();

   if(vr.useIOMatchCLR){
      vr_IOmatch_clr_init(vr.IOMatchScript);
   }
   vr_resetCount();

   if(vr.sourceExcludeActivated){
     if(!vr_includeSourceMutuallyExclusive(vr.includeSource, vr.excludeSourceRead)){
       VG_(tool_panic)("--source and --warn-unknown-source are incompatible");
     }
   }
   //Random Seed initialisation
   if(vr.firstSeed==(ULong )(-1)){
      struct vki_timeval now;
      VG_(gettimeofday)(&now, NULL);
      ULong pid = VG_(getpid)();
      vr.firstSeed = now.tv_usec + pid;
   }
   VG_(umsg)("First seed : %llu\n", vr.firstSeed);

   //deactivate interlibm LD_PRELOAD
   if(! vr.loadInterLibm){
     //adaptation from ms_mainc.c in massif tool
     HChar* s1;
     HChar* s2;
     HChar* LD_PRELOAD_val;

     LD_PRELOAD_val = VG_(getenv)( VG_(LD_PRELOAD_var_name) );
     tl_assert(LD_PRELOAD_val);

     // Make sure the vgpreload_core-$PLATFORM entry is there, for sanity.
     s1 = VG_(strstr)(LD_PRELOAD_val, "vgpreload_core");
     tl_assert(s1);

     // Now find the vgpreload_verrou-$PLATFORM entry.
     s1 = VG_(strstr)(LD_PRELOAD_val, "vgpreload_verrou");
     tl_assert(s1);
     s2 = s1;

     // Position s1 on the previous ':', which must be there because
     // of the preceding vgpreload_core-$PLATFORM entry.
     for (; *s1 != ':'; s1--)
       ;

     // Position s2 on the next ':' or \0
     for (; *s2 != ':' && *s2 != '\0'; s2++)
       ;

     // Move all characters from s2 to s1
     while ((*s1++ = *s2++))
       ;
   }//end deactivate LD_PRELOAD


   if(vr.prandomUpdate==VR_PRANDOM_UPDATE_FUNC){
     if( (vr.roundingMode==VR_PRANDOM_DET ||  vr.roundingMode==VR_PRANDOM_COMDET)){
       VG_(tool_panic)("prandom dynamic update and PRANDOM_[COM]DET are incompatible");
     }
   }

   //Verrou Backend Initilisation
   backend_verrou=interflop_verrou_init(&backend_verrou_context);
   backend_verrou_null=interflop_verrou_init(&backend_verrou_null_context);
   verrou_set_panic_handler(&VG_(tool_panic));

   verrou_set_nan_handler(&vr_handle_NaN);
   verrou_set_inf_handler(&vr_handle_Inf);

#ifdef PROFILING_EXACT
   verrou_init_profiling_exact();
#endif

   verrou_set_debug_print_op(&print_op);//Use only verrou backend is configured to use it

   VG_(umsg)("Backend %s : %s\n", interflop_verrou_get_backend_name() , interflop_verrou_get_backend_version() );

   interflop_verrou_configure(vr.roundingMode,backend_verrou_context);
   verrou_set_seed (vr.firstSeed);

   if(vr.prandomFixedInitialValue!=-1.){
     verrou_updatep_prandom_double(vr.prandomFixedInitialValue);
   }


   /*configuration of MCA backend*/
#ifdef USE_VERROU_QUADMATH
   backend_mcaquad=interflop_mcaquad_init(&backend_mcaquad_context);
   mcaquad_set_panic_handler(&VG_(tool_panic));

   VG_(umsg)("Backend %s : %s\n", interflop_mcaquad_get_backend_name(), interflop_mcaquad_get_backend_version() );


   mcaquad_conf_t mca_quad_conf;
   mca_quad_conf.precision_float=vr.mca_precision_float;
   mca_quad_conf.precision_double=vr.mca_precision_double;
   mca_quad_conf.mode=vr.mca_mode;
   interflop_mcaquad_configure(mca_quad_conf, backend_mcaquad_context);
   mcaquad_set_seed(vr.firstSeed);
#endif

   /*Init outfile cancellation*/
   checkcancellation_conf_t checkcancellation_conf;
   checkcancellation_conf.threshold_float= vr.cc_threshold_float;
   checkcancellation_conf.threshold_double= vr.cc_threshold_double;
   backend_checkcancellation=interflop_checkcancellation_init(&backend_checkcancellation_context);
   interflop_checkcancellation_configure(checkcancellation_conf,backend_checkcancellation_context);


   checkcancellation_set_cancellation_handler(&vr_handle_CC); //valgrind error

   VG_(umsg)("Backend %s : %s\n", interflop_checkcancellation_get_backend_name(), interflop_checkcancellation_get_backend_version() );



   backend_check_float_max=interflop_check_float_max_init(&backend_check_float_max_context);
   ifmax_set_max_handler(&vr_handle_FLT_MAX);
   ifmax_set_debug_print_op(&print_op);//Use only verrou backend is configured to use it

   VG_(umsg)("Backend %s : %s\n", interflop_check_float_max_get_backend_name(), interflop_check_float_max_get_backend_version() );



   /*Init outfile cancellation*/
   if(vr.roundingMode==VR_FTZ){
      vr.ftz=True;
   }

   checkdenorm_conf_t checkdenorm_conf;
   checkdenorm_conf.flushtozero= vr.ftz;
   backend_checkdenorm=interflop_checkdenorm_init(&backend_checkdenorm_context);
   interflop_checkdenorm_configure(checkdenorm_conf,backend_checkdenorm_context);
   checkdenorm_set_denorm_handler(&vr_handle_CD);
   checkdenorm_set_panic_handler(&VG_(tool_panic));
   VG_(umsg)("Backend %s : %s\n", interflop_checkdenorm_get_backend_name(), interflop_checkdenorm_get_backend_version()  );

  if( vr.checkDenorm || vr.dumpDenorm || vr.ftz){
     if( vr.backend==vr_mcaquad ||
         (vr.backend==vr_verrou && !(vr.roundingMode==VR_NEAREST || vr.roundingMode==VR_FTZ || vr.roundingMode==VR_NATIVE))){
        VG_(tool_panic)("backend checkDenorm incompatible with other backend");
     }
     vr.backend=vr_checkdenorm;
  }
  if(vr.checkFloatMax && vr.backend!=vr_verrou){
    VG_(tool_panic)("backend check_float_max is only compatible with verrou backend");
  }

  if(vr.checkFloatMax && vr.float_conv){
    VG_(tool_panic)("--float and --checkFloatMax are incompatible");
  }


  if( vr.loadInterLibm && (vr.backend!=vr_verrou) ){
    VG_(tool_panic)("--libm=instrumented  is compatible only with verrou backend");
  }

  if(vr.genTrace){
     vr_traceBB_initialize(vr.outputTraceRep);
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
#ifndef USE_VERROU_FMA
     vr.instr_op[VR_OP_MADD]=False;
     vr.instr_op[VR_OP_MSUB]=False;
#endif
#ifndef USE_VERROU_SQRT
     vr.instr_op[VR_OP_SQRT]=False;
#endif
     vr.instr_op[VR_OP_CONV]=True;
   }
   VG_(umsg)("Instrumented operations :\n");
   for (opIt=0; opIt< VR_OP ;opIt++){
     VG_(umsg)("\t%s : ", vr_ppOp(opIt));
     if(vr.instr_op[opIt]==True) VG_(umsg)("yes\n");
     else VG_(umsg)("no\n");
   }
   VG_(umsg)("Instrumented vectorized operations :\n");
   int vecIt;
   for (vecIt=0; vecIt< VR_VEC ;vecIt++){
      VG_(umsg)("\t%s : ", vr_ppVec(vecIt));
      if(vr.instr_vec[vecIt]==True) VG_(umsg)("yes\n");
      else VG_(umsg)("no\n");
   }

   VG_(umsg)("Instrumented type :\n");
   int precIt;
   int nbPrec=2;
   if(vr.loadInterLibm){
     nbPrec=3;
   }
   for (precIt=0; precIt< nbPrec ;precIt++){
      VG_(umsg)("\t%s : ", vr_ppPrec(precIt));
      if(vr.instr_prec[precIt]==True) VG_(umsg)("yes\n");
      else VG_(umsg)("no\n");
   }
   if(vr.float_conv){
      VG_(umsg)("Frontend: double -> float\n");
   }

   if(vr.backend==vr_verrou){
      VG_(umsg)("Backend verrou simulating %s rounding mode\n", verrou_rounding_mode_name (vr.roundingMode));
      if(vr.roundingMode==VR_PRANDOM || vr.roundingMode==VR_PRANDOM_DET || vr.roundingMode==VR_PRANDOM_COMDET){
	VG_(umsg)("\t PRANDOM: pvalue=%f\n", verrou_prandom_pvalue());
      }
   }
   if(vr.backend==vr_mcaquad){
#ifdef USE_VERROU_QUADMATH
     VG_(umsg)("Backend mcaquad simulating mode %s with precision %u for double and %u for float\n", mcaquad_mode_name(vr.mca_mode), vr.mca_precision_double, vr.mca_precision_float );
#else
     VG_(tool_panic)("Verrou compiled without quad support");
#endif
   }
   if(vr.sourceExcludeActivated && !(vr.sourceActivated)){
     VG_(tool_panic)("warn-unknown-source need --source(or VERROU_SOURCE) defined ");
   }
}

static
void vr_pre_syscall(ThreadId tid, UInt syscallno,
                    UWord* args, UInt nArgs){
}

#if defined(VGA_amd64)
#define VR_OPEN_AT_SYSCALLNO 257
#define VR_WRITE_SYSCALLNO 1
#define VR_OPEN_SYSCALLNO 2
#define VR_CLOSE_SYSCALLNO 3
#define VR_CREAT_SYSCALLNO 85

#elif defined(VGA_arm64)
#define VR_ARM64_UNDEF 255

#define VR_OPEN_AT_SYSCALLNO 56
#define VR_WRITE_SYSCALLNO 64
#define VR_OPEN_SYSCALLNO 255
#define VR_CLOSE_SYSCALLNO 57
#define VR_CREAT_SYSCALLNO 255
#else
#error "Unknown arch : syscallno need to be defined"
#endif
static
void vr_post_syscall(ThreadId tid, UInt syscallno,
                     UWord* args, UInt nArgs, SysRes res){
   if(vr.excludeDetect){
     if(syscallno==VR_OPEN_AT_SYSCALLNO){//openat
        SizeT fd= sr_Res(res);
        if(fd>0){
	 const HChar* buffer;
	 Bool resolved=VG_(resolve_filename)(fd,&buffer);
	 if(resolved){
       	   vr.exclude=vr_addObjectIfMatchPattern(vr.exclude, buffer);
	 }else{
	   vr.exclude=vr_addObjectIfMatchPattern(vr.exclude, (char*)args[1]);
	 }
       }
     }
   }

   if(vr.useIOMatchCLR){
      if((syscallno==VR_WRITE_SYSCALLNO) && (vr.IOMatchFileDescriptor !=-1)){//syscall write
         int fd=(int)args[0];
         if(fd==vr.IOMatchFileDescriptor){//sortie standard
            const HChar* buf=(HChar*)args[1];

            int size=(int)args[2];
            VG_(ok_to_discard_translations)=True; //I hope it's allowed to do that (required for stop and start call)
            vr_IOmatch_clr_checkmatch(buf,size);
            VG_(ok_to_discard_translations)=False;
         }
      }
      if(vr.outputIOMatchFilePattern!=NULL){
         if( (syscallno==VR_OPEN_AT_SYSCALLNO) || (syscallno==VR_OPEN_SYSCALLNO) || (syscallno==VR_CREAT_SYSCALLNO)){//syscall openat open creat
	  SizeT fd= sr_Res(res);;
	  char* filenameParam=NULL;
	  if(syscallno == VR_OPEN_AT_SYSCALLNO){//openat
	    filenameParam=(char*) args[1];
	  }
	  if( (syscallno==VR_OPEN_SYSCALLNO) || (syscallno==VR_CREAT_SYSCALLNO)){//open creat
	    filenameParam=(char*) args[0];
	  }

	  if(VG_(string_match)(vr.outputIOMatchFilePattern,filenameParam)){
	    if(vr.IOMatchFileDescriptor==-1){
	      vr.IOMatchFileDescriptor=fd;
	      HChar openMsg[512];
	      UInt size=VG_(snprintf)(openMsg,512, "IOMatch open filename: %s\n", filenameParam);
	      VG_(ok_to_discard_translations)=True; //I hope it's allowed to do that (required for stop and start call)
	      vr_IOmatch_clr_checkmatch(openMsg, size+1);
	      VG_(ok_to_discard_translations)=False;
	    }else{
	      VG_(umsg)("%s match several opened files : %s\n", vr.outputIOMatchFilePattern, filenameParam);
	      VG_(tool_panic)("match several opened files");
	    }
	  }
	  //VG_(umsg)("%s do not match %s\n", filename, vr.outputIOMatchFilePattern );
	}

	if(syscallno==VR_CLOSE_SYSCALLNO){//syscall close
	  int fd=(int)args[0];
	  if(fd==vr.IOMatchFileDescriptor ){
	    vr.IOMatchFileDescriptor=-1;//retour à la sortie standard.
	    HChar openMsg[]="IOMatch close\n";
	    vr_IOmatch_clr_checkmatch(openMsg, sizeof(openMsg));
	  }
	}
      }
   }
}

static void vr_pre_clo_init(void)
{
   VG_(details_name)            ("Verrou");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("Check floating-point rounding errors");
   VG_(details_copyright_author)(
      "Copyright (C) 2014-2019, EDF (F. Fevotte & B. Lathuiliere). 2019-2023, EDF (B. Lathuiliere). 2020-2021, TriScale innov (F. Fevotte)\n ");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );


   VG_(clo_vex_control).iropt_register_updates_default
     = VG_(clo_px_file_backed)
     = VexRegUpdSpAtMemAccess; // overridable by the user.

   //VG_(clo_vex_control).iropt_unroll_thresh = 0;   // cannot be overriden.

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

   VG_(needs_syscall_wrapper)(vr_pre_syscall,
                              vr_post_syscall);

   vr_clo_defaults();
}

VG_DETERMINE_INTERFACE_VERSION(vr_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
