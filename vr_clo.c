
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code related command-line options.        ---*/
/*---                                                     vr_clo.c ---*/
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
//#include "backend_verrou/vr_rand.h"
//#include "backend_verrou/interflop_verrou.h"
//#include "backend_mcaquad/interflop_mcaquad.h"

void vr_env_clo (const HChar* env, const HChar *clo) {
  HChar* val = VG_(getenv)(env);
  if (val) {
    HChar tmp[256];
    VG_(snprintf)(tmp, 255, "%s=%s", clo, val);
    if (!vr_process_clo(tmp)) {
      VG_(umsg)("WARNING: unknown command-line option `%s'\n", tmp);
    }
  }
}

void vr_clo_defaults (void) {
  vr.backend = vr_verrou;
  vr.roundingMode = VR_NEAREST;
  vr.count = True;
  vr.instrument = VR_INSTR_ON;
  vr.verbose = False;
  vr.unsafe_llo_optim = False;

  vr.genExcludeBool = False;
  vr.exclude = NULL;
  vr.gen_exclude = NULL;
  //  vr.genAbove = NULL;

  vr.genIncludeSource = False;
  vr.includeSource = NULL;
  vr.sourceActivated= False;
  vr.excludeSourceRead = NULL;
  vr.excludeSourceDyn = NULL;
  vr.sourceExcludeActivated = False;
  vr.genTrace=False;
  vr.includeTrace = NULL;
  vr.outputTraceRep = NULL;

  int opIt;
  for(opIt=0 ; opIt<VR_OP ; opIt++){
    vr.instr_op[opIt]=False;
  }
  int vecIt;
  for(vecIt=0 ; vecIt<VR_VEC ; vecIt++){
    vr.instr_vec[vecIt]=True;
  }
  vr.instr_vec[VR_VEC_SCAL]=False;

  vr.instr_prec[VR_PREC_FLT]=True;
  vr.instr_prec[VR_PREC_DBL]=True;
  vr.instr_prec[VR_PREC_DBL_TO_FLT]=True;

  vr.firstSeed=(ULong)(-1);
  vr.mca_precision_double=53;
  vr.mca_precision_float=24;
  vr.mca_mode=MCAMODE_MCA;

  vr.checknan=True;
  vr.checkinf=True;
  
  vr.checkCancellation=False;
  vr.cc_threshold_float=18;
  vr.cc_threshold_double=40;

  vr.dumpCancellation=False;
  vr.cancellationSource=NULL;

  vr.checkDenorm=False;
  vr.ftz=False;
  vr.dumpDenorm=False;
  vr.cancellationSource=NULL;

  vr.checkFloatMax=False;
}


Bool vr_process_clo (const HChar *arg) {
  Bool bool_val;
  const HChar * str;
  UInt setResult;

  //Option --backend=
  if      (VG_XACT_CLOM (cloPD, arg, "--backend=verrou",
                         vr.backend, vr_verrou)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--backend=mcaquad",
                         vr.backend, vr_mcaquad)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--backend=checkdenorm",
                         vr.backend, vr_checkdenorm)) {}

  //Option --rounding-mode=
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random",
                         vr.roundingMode, VR_RANDOM)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random_det",
                         vr.roundingMode, VR_RANDOM_DET)) {}
    else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random_comdet",
                         vr.roundingMode, VR_RANDOM_COMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average",
                         vr.roundingMode, VR_AVERAGE)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average_det",
                         vr.roundingMode, VR_AVERAGE_DET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average_comdet",
                         vr.roundingMode, VR_AVERAGE_COMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=nearest",
                         vr.roundingMode, VR_NEAREST)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=upward",
                         vr.roundingMode, VR_UPWARD)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=downward",
                         vr.roundingMode, VR_DOWNWARD)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=toward_zero",
                         vr.roundingMode, VR_ZERO)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=farthest",
                         vr.roundingMode, VR_FARTHEST)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=float",
                         vr.roundingMode, VR_FLOAT)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=native",
                         vr.roundingMode, VR_NATIVE)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=ftz",
                         vr.roundingMode, VR_FTZ)) {}

  //Option mcaquad
  else if (VG_INT_CLOM  (cloPD, arg, "--mca-precision-double",
                         vr.mca_precision_double)){}
  else if (VG_INT_CLOM  (cloPD, arg, "--mca-precision-float",
                         vr.mca_precision_float)){}
  else if (VG_XACT_CLOM (cloPD, arg, "--mca-mode=rr",
                         vr.mca_mode, MCAMODE_RR)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--mca-mode=pb",
                         vr.mca_mode, MCAMODE_PB)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--mca-mode=mca",
                         vr.mca_mode, MCAMODE_MCA)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--mca-mode=ieee",
                         vr.mca_mode, MCAMODE_IEEE)) {}

  //Option checkdenorm
  else if (VG_BOOL_CLO (arg, "--check-denorm", bool_val)) {
     vr.checkDenorm= bool_val;
  }

  //Options to choose op to instrument
  else if (VG_USET_CLOM(cloPD, arg, "--vr-instr", "add,sub,mul,div,mAdd,mSub,conv", setResult)){
    UInt instrTab[]={0,0,0,0,0,0,0};
    UInt currentFlags=setResult;
    for(UInt i=0; i<7;i++){
      instrTab[i]=currentFlags%2;
      currentFlags=currentFlags/2;
    }
    if(instrTab[0]!=0) vr.instr_op[VR_OP_ADD]=True;
    if(instrTab[1]!=0) vr.instr_op[VR_OP_SUB]=True;
    if(instrTab[2]!=0) vr.instr_op[VR_OP_MUL]=True;
    if(instrTab[3]!=0) vr.instr_op[VR_OP_DIV]=True;
    if(instrTab[4]!=0) vr.instr_op[VR_OP_MADD]=True;
    if(instrTab[5]!=0) vr.instr_op[VR_OP_MSUB]=True;
    if(instrTab[6]!=0) vr.instr_op[VR_OP_CONV]=True;
  }

  //Option to enable check-cancellation backend
  else if (VG_BOOL_CLO (arg, "--check-cancellation", bool_val)) {
     vr.checkCancellation= bool_val;
  }
  else if (VG_INT_CLO (arg, "--cc-threshold-double",
                       vr.cc_threshold_double)){}
  else if (VG_INT_CLO (arg, "--cc-threshold-float",
                       vr.cc_threshold_float)){}

  else if (VG_BOOL_CLO (arg, "--check-nan", bool_val)) {
     vr.checknan= bool_val;
  }
  else if (VG_BOOL_CLO (arg, "--check-inf", bool_val)) {
     vr.checkinf= bool_val;
  }

  
  else if (VG_BOOL_CLO (arg, "--check-max-float", bool_val)) {
    vr.checkFloatMax=bool_val;
  }

  //Options to choose op to instrument
  else if (VG_BOOL_CLO (arg, "--vr-instr-scalar", bool_val)) {
    vr.instr_vec[VR_VEC_SCAL]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-llo", bool_val)) {
    vr.instr_vec[VR_VEC_LLO]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-vec2", bool_val)) {
    vr.instr_vec[VR_VEC_FULL2]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-vec4", bool_val)) {
     vr.instr_vec[VR_VEC_FULL4]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-vec8", bool_val)) {
     vr.instr_vec[VR_VEC_FULL8]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-flt", bool_val)) {
     vr.instr_prec[VR_PREC_FLT]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-dbl", bool_val)) {
     vr.instr_prec[VR_PREC_DBL]= bool_val;
  }

  //Option --vr-verbose (to avoid verbose of valgrind)
  else if (VG_BOOL_CLO (arg, "--vr-verbose", bool_val)) {
    vr.verbose = bool_val;
  }

  //Option --vr-unsafe-llo-optim (performance optimization)
  else if (VG_BOOL_CLO (arg, "--vr-unsafe-llo-optim", bool_val)) {
    vr.unsafe_llo_optim = bool_val;
  }

  //Option --count-op
  else if (VG_BOOL_CLO (arg, "--count-op", bool_val)) {
    vr.count = bool_val;
  }

  // Instrumentation at start
  else if (VG_BOOL_CLOM (cloPD, arg, "--instr-atstart", bool_val)) {
    vr.instrument = bool_val ? VR_INSTR_ON : VR_INSTR_OFF;
  }

  // Exclusion of specified symbols
  else if (VG_STR_CLOM (cloPD, arg, "--gen-exclude", str)) {
    //vr.excludeFile = VG_(strdup)("vr.process_clo.gen-exclude", str);
    vr.excludeFile = VG_(expand_file_name)("vr.process_clo.gen-exclude", str);
    vr.genExcludeBool = True;
  }
  /* else if (VG_STR_CLOM (cloPD, arg, "--gen-above", str)) { */
  /*   vr.genAbove = VG_(strdup)("vr.process_clo.gen-above", str); */
  /* } */
  else if (VG_STR_CLOM (cloPD, arg, "--exclude", str)) {
    vr.exclude = vr_loadExcludeList(vr.exclude, str);
  }

  else if (VG_STR_CLOM  (cloPD, arg, "--trace", str)) {
    vr.includeTrace = vr_loadIncludeTraceList(vr.includeTrace, str);
    vr.genTrace = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--output-trace-rep", str)) {
    //vr.includeSourceFile = VG_(strdup)("vr.process_clo.gen-source", str);
    vr.outputTraceRep = VG_(expand_file_name)("vr.process_clo.trace-rep", str);
  }
  // Instrumentation of only specified source lines
  else if (VG_STR_CLOM (cloPD, arg, "--gen-source", str)) {
    //vr.includeSourceFile = VG_(strdup)("vr.process_clo.gen-source", str);
    vr.includeSourceFile = VG_(expand_file_name)("vr.process_clo.gen-source", str);
    vr.genIncludeSource = True;
  }
  else if (VG_STR_CLOM (cloPD, arg, "--source", str)) {
    vr.includeSource = vr_loadIncludeSourceList(vr.includeSource, str);
    vr.sourceActivated = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--warn-unknown-source", str)) {
    vr.excludeSourceRead = vr_loadIncludeSourceList(vr.excludeSourceRead, str);
    vr.excludeSourceDyn=vr.excludeSourceRead;
    vr.sourceExcludeActivated = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--cc-gen-file", str)) {
     vr.cancellationDumpFile = VG_(expand_file_name)("vr.process_clo.cc-file", str);
     vr.dumpCancellation = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--cd-gen-file", str)) {
     vr.denormDumpFile = VG_(expand_file_name)("vr.process_clo.cd-file", str);
     vr.dumpDenorm = True;
  }


  // Set the pseudo-Random Number Generator
  else if (VG_STR_CLOM (cloPD, arg, "--vr-seed", str)) {
    //vr_rand_setSeed (&vr_rand, VG_(strtoull10)(str, NULL));
    vr.firstSeed=VG_(strtoull10)(str, NULL);
    if(vr.firstSeed==(ULong)(-1)){
      VG_(tool_panic) ( "--vr-seed=-1 no taken into account\n");
    }
  }

  // Unknown option
  else {
    return False;
  }

  return True;
}

void vr_print_usage (void) {
  VG_(printf)
    (
#include "vr_clo.txt"
);
}

void vr_print_debug_usage (void) {
  vr_print_usage();
}
