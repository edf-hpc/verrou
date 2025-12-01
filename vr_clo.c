
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

#include "vr_clo.h"


void vr_env_clo_one_option (const HChar* env, const HChar *clo) {
  HChar* val = VG_(getenv)(env);
  if (val) {
    HChar tmp[256];
    VG_(snprintf)(tmp, 255, "%s=%s", clo, val);
    if (!vr_process_clo(tmp)) {
      VG_(umsg)("WARNING: unknown command-line option `%s'\n", tmp);
    }
  }
}


void vr_env_clo(void){
   vr_env_clo_one_option("VERROU_ROUNDING_MODE", "--rounding-mode");
   vr_env_clo_one_option("VERROU_FLOAT", "--float");
   vr_env_clo_one_option("VERROU_UNFUSED", "--unfused");
   vr_env_clo_one_option("VERROU_LIBM_NOINST_ROUNDING_MODE", "--libm-noinst-rounding-mode");
   vr_env_clo_one_option("VERROU_PRANDOM_UPDATE", "--prandom-update");
   vr_env_clo_one_option("VERROU_PRANDOM_PVALUE", "--prandom-pvalue");
   vr_env_clo_one_option("VERROU_INSTR_ATSTART", "--instr-atstart");
   vr_env_clo_one_option("VERROU_INSTR_ATSTART_SOFT", "--instr-atstart-soft");
   vr_env_clo_one_option("VERROU_EXCLUDE",       "--exclude");
   vr_env_clo_one_option("VERROU_GEN_EXCLUDE",   "--gen-exclude");

   vr_env_clo_one_option("VERROU_EXCLUDE_BACKTRACE", "--exclude-backtrace");
   vr_env_clo_one_option("VERROU_GEN_BACKTRACE",   "--gen-backtrace");

   vr_env_clo_one_option("VERROU_SOURCE",        "--source");
   vr_env_clo_one_option("VERROU_WARN_UNKNOWN_SOURCE","--warn-unknown-source");
   vr_env_clo_one_option("VERROU_GEN_SOURCE",    "--gen-source");
   vr_env_clo_one_option("VERROU_MCA_MODE",      "--mca-mode");

   vr_env_clo_one_option("VERROU_BACKEND", "--backend");
   vr_env_clo_one_option("VERROU_MCA_PRECISION_DOUBLE", "--mca-precision-double");
   vr_env_clo_one_option("VERROU_MCA_PRECISION_FLOAT", "--mca-precision-float");

   vr_env_clo_one_option("VERROU_INSTR","--vr-instr");

   vr_env_clo_one_option("VERROU_TRACE","--trace");
   vr_env_clo_one_option("VERROU_OUTPUT_TRACE_REP","--output-trace-rep");
   vr_env_clo_one_option("VERROU_SEED","--vr-seed");

   vr_env_clo_one_option("VERROU_IOMATCH_CLR","--IOmatch-clr");
   vr_env_clo_one_option("VERROU_OUTPUT_IOMATCH_REP","--output-IOmatch-rep");
   vr_env_clo_one_option("VERROU_IOMATCH_FILE_PATTERN","--IOmatch-file-pattern");

   vr_env_clo_one_option("VERROU_COUNT_OP","--count-op");
   vr_env_clo_one_option("VERROU_COUNT_DENORM","--count-denorm");


   vr_env_clo_one_option("VERROU_VPREC_PRECISION_BINARY64", "--vprec-precision-binary64");
   vr_env_clo_one_option("VERROU_VPREC_RANGE_BINARY64", "--vprec-range-binary64");
   vr_env_clo_one_option("VERROU_VPREC_PRECISION_BINARY32", "--vprec-precision-binary32");
   vr_env_clo_one_option("VERROU_VPREC_RANGE_BINARY32", "--vprec-range-binary32");
   vr_env_clo_one_option("VERROU_VPREC_MODE", "--vprec-mode");
   vr_env_clo_one_option("VERROU_VPREC_PRESET", "--vprec-preset");
   vr_env_clo_one_option("VERROU_VPREC_ERROR_MODE", "--vprec-error-mode");
   vr_env_clo_one_option("VERROU_VPREC_MAX_ERROR_EXPONENT","--vprec-max-abs-error-exponent");
   vr_env_clo_one_option("VERROU_VPREC_DAZ", "--vprec-daz");
   vr_env_clo_one_option("VERROU_VPREC_FTZ", "--vprec-ftz");
}

void vr_clo_defaults (void) {
  vr.backend = vr_verrou;
  vr.roundingMode = VR_NEAREST;
  vr.roundingModeNoInst = VR_NATIVE;
  vr.prandomUpdate= VR_PRANDOM_UPDATE_NONE;
  vr.prandomFixedInitialValue=-1.;
  vr.count = True;

  vr.float_conv=False;
  vr.unfused=False;
  vr.instrument_hard = VR_INSTR_ON;
  vr.instrument_soft = VR_INSTR_ON;
  vr.instrument_soft_back = VR_INSTR_ON;

  vr.instrument_soft_used = False;
  vr.instrument_soft_back_used =False;

  vr.verbose = False;

  vr.genExcludeBool = False;
  vr.exclude = NULL;
  vr.gen_exclude = NULL;

  vr.excludeDetect = True;
  vr.loadInterLibm = False;
  vr.excludePython = True;

  vr.genIncludeSource = False;
  vr.includeSource = NULL;
  vr.sourceActivated= False;
  vr.excludeSourceRead = NULL;
  vr.sourceExcludeActivated = False;

  vr.genBackTraceBool=False;
  vr.useBackTraceBool=False;
  vr.genBackTraceRep=NULL;
  vr.backExcludeFile=NULL;
  vr.backIgnoreSize=0;

  vr.genTrace=False;
  vr.includeTrace = NULL;
  vr.outputTraceRep = NULL;
  vr.outputIOMatchRep = NULL;
  vr.outputIOMatchFilePattern= NULL;
  vr.IOMatchFileDescriptor=1;//stdout

  int opIt;
  for(opIt=0 ; opIt<VR_OP ; opIt++){
    vr.instr_op[opIt]=False;
  }
  int vecIt;
  for(vecIt=0 ; vecIt<VR_VEC ; vecIt++){
    vr.instr_vec[vecIt]=True;
  }
  vr.instr_vec[VR_VEC_SCAL]=False;
#if defined(VGA_arm64)
  vr.instr_vec[VR_VEC_SCAL]=True;
#endif
  vr.instr_prec[VR_PREC_FLT]=True;
  vr.instr_prec[VR_PREC_DBL]=True;
  vr.instr_prec[VR_PREC_LDBL]=False;
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

  vr.useIOMatchCLR=False;
  vr.IOMatchCLRFileInput=-1;

  vr.checkDenorm=False;
  vr.ftz=False;
  vr.daz=False;
  vr.dumpDenormOut=False;
  vr.dumpDenormIn=False;
  vr.counterDenorm=False;

  vr.cancellationSource=NULL;
  vr.checkFloatMax=False;

  vr.vprec_conf.preset = (UInt)(-1);
  vr.vprec_conf.precision_binary32 = VPREC_PRECISION_BINARY32_DEFAULT;
  vr.vprec_conf.range_binary32 = VPREC_RANGE_BINARY32_DEFAULT;
  vr.vprec_conf.precision_binary64 = VPREC_PRECISION_BINARY64_DEFAULT;
  vr.vprec_conf.range_binary64 = VPREC_RANGE_BINARY64_DEFAULT;
  vr.vprec_conf.mode = VPREC_MODE_DEFAULT;
  vr.vprec_conf.err_mode =  vprec_err_mode_rel;
  vr.vprec_conf.max_abs_err_exponent = -DOUBLE_EXP_MIN;
  vr.vprec_conf.daz = False;
  vr.vprec_conf.ftz = False;

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
    else if (VG_XACT_CLOM (cloPD, arg, "--backend=vprec",
                         vr.backend, vr_vprec)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--backend=checkdenorm",
                         vr.backend, vr_checkdenorm)) {}

  //Option --rounding-mode=
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random",
                         vr.roundingMode, VR_RANDOM)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random_det",
                         vr.roundingMode, VR_RANDOM_DET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random_comdet",
                         vr.roundingMode, VR_RANDOM_COMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=random_scomdet",
                         vr.roundingMode, VR_RANDOM_SCOMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=sr_monotonic",
                         vr.roundingMode, VR_SR_MONOTONIC)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=sr_smonotonic",
                         vr.roundingMode, VR_SR_SMONOTONIC)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average",
                         vr.roundingMode, VR_AVERAGE)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average_det",
                         vr.roundingMode, VR_AVERAGE_DET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average_comdet",
                         vr.roundingMode, VR_AVERAGE_COMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=average_scomdet",
                         vr.roundingMode, VR_AVERAGE_SCOMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=prandom",
                         vr.roundingMode, VR_PRANDOM)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=prandom_det",
                         vr.roundingMode, VR_PRANDOM_DET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=prandom_comdet",
                         vr.roundingMode, VR_PRANDOM_COMDET)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=nearest",
                         vr.roundingMode, VR_NEAREST)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=upward",
                         vr.roundingMode, VR_UPWARD)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=downward",
                         vr.roundingMode, VR_DOWNWARD)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=toward_zero",
                         vr.roundingMode, VR_ZERO)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=away_zero",
                         vr.roundingMode, VR_AWAY_ZERO)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=farthest",
                         vr.roundingMode, VR_FARTHEST)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=float",
                         vr.roundingMode, VR_FLOAT)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=native",
                         vr.roundingMode, VR_NATIVE)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=ftz",
                         vr.roundingMode, VR_FTZ)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=daz",
                         vr.roundingMode, VR_DAZ)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--rounding-mode=dazftz",
                         vr.roundingMode, VR_DAZFTZ)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--libm-noinst-rounding-mode=nearest",
                         vr.roundingModeNoInst, VR_NEAREST)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--libm-noinst-rounding-mode=native",
                         vr.roundingModeNoInst, VR_NATIVE)) {}

  else if (VG_XACT_CLOM (cloPD, arg, "--prandom-update=func",
                         vr.prandomUpdate, VR_PRANDOM_UPDATE_FUNC)) {}
  else if (VG_XACT_CLOM (cloPD, arg, "--prandom-update=none",
                         vr.prandomUpdate, VR_PRANDOM_UPDATE_NONE)) {}

  else if (VG_STR_CLOM (cloPD, arg, "--prandom-pvalue", str)) {
    vr.prandomFixedInitialValue=VG_(strtod)(str, NULL);
    if(vr.prandomFixedInitialValue<0 ||  vr.prandomFixedInitialValue>1){
      VG_(tool_panic) ( "\tpvalue has to be between 0 and 1\n");
    }
  }
  else if (VG_BOOL_CLOM (cloPD, arg, "--float", bool_val)) {
    vr.float_conv = bool_val;
  }
  else if (VG_BOOL_CLOM (cloPD, arg, "--unfused", bool_val)) {
    vr.unfused = bool_val;
  }

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
  else if (VG_USET_CLOM(cloPD, arg, "--vr-instr", "add,sub,mul,div,mAdd,mSub,conv,sqrt", setResult)){
    UInt instrTab[]={0,0,0,0,0,0,0,0};
    UInt currentFlags=setResult;
    for(UInt i=0; i<8;i++){
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
    if(instrTab[7]!=0) vr.instr_op[VR_OP_SQRT]=True;
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
  else if (VG_BOOL_CLO (arg, "--vr-instr-unk", bool_val)) {
     vr.instr_vec[VR_VEC_UNK]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-flt", bool_val)) {
     vr.instr_prec[VR_PREC_FLT]= bool_val;
  }

  else if (VG_BOOL_CLO (arg, "--vr-instr-dbl", bool_val)) {
     vr.instr_prec[VR_PREC_DBL]= bool_val;
  }
  /* else if (VG_BOOL_CLO (arg, "--vr-instr-ldbl", bool_val)) { */
  /*    vr.instr_prec[VR_PREC_LDBL]= bool_val; */
  /* } */

  //Option --vr-verbose (to avoid verbose of valgrind)
  else if (VG_BOOL_CLOM (cloPD, arg, "--vr-verbose", bool_val)) {
    vr.verbose = bool_val;
  }

  //Option --count-op
  else if (VG_BOOL_CLOM (cloPD, arg, "--count-op", bool_val)) {
    vr.count = bool_val;
  }

  // Instrumentation at start
  else if (VG_BOOL_CLOM (cloPD, arg, "--instr-atstart", bool_val)) {
    vr.instrument_hard = bool_val ? VR_INSTR_ON : VR_INSTR_OFF;
  }
  else if (VG_BOOL_CLOM (cloPD, arg, "--instr-atstart-soft", bool_val)) {
    vr.instrument_soft = bool_val ? VR_INSTR_ON : VR_INSTR_OFF;
    vr.instrument_soft_used= True;
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

  else if (VG_XACT_CLOM (cloPD, arg, "--libm=auto_exclude", vr.excludeDetect,True)) {
    vr.loadInterLibm = False;
  }
  else if (VG_XACT_CLOM (cloPD, arg, "--libm=manual_exclude", vr.excludeDetect,False)) {
    vr.loadInterLibm = False;
  }
  else if (VG_XACT_CLOM (cloPD, arg, "--libm=instrumented", vr.excludeDetect,True)) {
    vr.loadInterLibm = True;
  }
  else if (VG_XACT_CLOM (cloPD, arg, "--python=auto_exclude", vr.excludePython,True)) {
  }
  else if (VG_XACT_CLOM (cloPD, arg, "--python=manual_exclude", vr.excludePython,False)) {
  }

  else if (VG_STR_CLOM  (cloPD, arg, "--trace", str)) {
    vr.includeTrace = vr_loadIncludeTraceList(vr.includeTrace, str);
    vr.genTrace = True;
  }

  else if (VG_STR_CLOM  (cloPD, arg, "--gen-backtrace", str)) {
    vr.genBackTraceRep = VG_(expand_file_name)("vr.process_clo.back-rep", str);
    vr.genBackTraceBool = True;
  }
  else if (VG_STR_CLOM  (cloPD, arg, "--exclude-backtrace", str)) {
    vr.backExcludeFile = VG_(expand_file_name)("vr.process_clo.backtrace", str);
    if(vr.backExcludeFile==NULL){
       VG_(umsg)("problem backExcludeFile\n");
    }
    vr.useBackTraceBool = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--output-trace-rep", str)) {
    vr.outputTraceRep = VG_(expand_file_name)("vr.process_clo.trace-rep", str);
  }

  // Instrumentation of only specified source lines
  else if (VG_STR_CLOM (cloPD, arg, "--gen-source", str)) {
    vr.includeSourceFile = VG_(expand_file_name)("vr.process_clo.gen-source", str);
    vr.genIncludeSource = True;
  }
  else if (VG_STR_CLOM (cloPD, arg, "--source", str)) {
    vr.includeSource = vr_loadIncludeSourceList(vr.includeSource, str);
    vr.sourceActivated = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--warn-unknown-source", str)) {
    vr.excludeSourceRead = vr_loadIncludeSourceList(vr.excludeSourceRead, str);
    vr.sourceExcludeActivated = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--cc-gen-file", str)) {
     vr.cancellationDumpFile = VG_(expand_file_name)("vr.process_clo.cc-file", str);
     vr.dumpCancellation = True;
  }

  else if (VG_STR_CLOM (cloPD, arg, "--cdi-gen-file", str)) {
     vr.denormInputDumpFile = VG_(expand_file_name)("vr.process_clo.cd-file", str);
     vr.dumpDenormIn = True;
  }
  else if (VG_STR_CLOM (cloPD, arg, "--cdo-gen-file", str)) {
     vr.denormOutputDumpFile = VG_(expand_file_name)("vr.process_clo.cd-file", str);
     vr.dumpDenormOut = True;
  }
  else if (VG_BOOL_CLOM (cloPD, arg, "--count-denorm", bool_val)) {
    vr.counterDenorm = bool_val;
  }



  // Set the pseudo-Random Number Generator
  else if (VG_STR_CLOM (cloPD, arg, "--vr-seed", str)) {
    //vr_rand_setSeed (&vr_rand, VG_(strtoull10)(str, NULL));
    vr.firstSeed=VG_(strtoull10)(str, NULL);
    if(vr.firstSeed==(ULong)(-1)){
      VG_(tool_panic) ( "--vr-seed=-1 no taken into account\n");
    }
  }
  else if (VG_STR_CLOM(cloPD, arg, "--IOmatch-clr",str)){
     vr.IOMatchScript = VG_(expand_file_name)("vr.process_clo.IOMatch-clr", str);
     vr.useIOMatchCLR=True;
  }
  else if (VG_STR_CLOM (cloPD, arg, "--output-IOmatch-rep", str)) {
    vr.outputIOMatchRep = VG_(expand_file_name)("vr.process_clo.IOMatch-rep", str);
  }
  else if (VG_STR_CLOM (cloPD, arg, "--IOmatch-file-pattern", str)) {
    vr.IOMatchFileDescriptor=-1;
    vr.outputIOMatchFilePattern = VG_(strdup)("vr.process_clo.IOMatch-file-pattern", str);
  }

    // Options VPREC
  else if (VG_INT_CLOM(cloPD, arg, "--vprec-precision-binary64",
                       vr.vprec_conf.precision_binary64)) {
  } else if (VG_INT_CLOM(cloPD, arg, "--vprec-precision-binary32",
                         vr.vprec_conf.precision_binary32)) {
  } else if (VG_INT_CLOM(cloPD, arg, "--vprec-range-binary64",
                         vr.vprec_conf.range_binary64)) {
  } else if (VG_INT_CLOM(cloPD, arg, "--vprec-range-binary32",
                         vr.vprec_conf.range_binary32)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-mode=ob", vr.vprec_conf.mode,
                          vprecmode_ob)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-mode=ib", vr.vprec_conf.mode,
                          vprecmode_ib)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-mode=full", vr.vprec_conf.mode,
                          vprecmode_full)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-mode=ieee", vr.vprec_conf.mode,
                          vprecmode_ieee)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=binary16",
                          vr.vprec_conf.preset, vprec_preset_binary16)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=binary32",
                          vr.vprec_conf.preset, vprec_preset_binary32)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=bfloat16",
                          vr.vprec_conf.preset, vprec_preset_bfloat16)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=tensorfloat",
                          vr.vprec_conf.preset, vprec_preset_tensorfloat)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=fp24", vr.vprec_conf.preset,
                          vprec_preset_fp24)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-preset=PXR24", vr.vprec_conf.preset,
                          vprec_preset_PXR24)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-error-mode=rel",
                          vr.vprec_conf.err_mode, vprec_err_mode_rel)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-error-mode=abs",
                          vr.vprec_conf.err_mode, vprec_err_mode_abs)) {
  } else if (VG_XACT_CLOM(cloPD, arg, "--vprec-error-mode=all",
                          vr.vprec_conf.err_mode, vprec_err_mode_all)) {
  } else if (VG_INT_CLOM(cloPD, arg, "--vprec-max-abs-error-exponent",
                         vr.vprec_conf.max_abs_err_exponent)) {
  } else if (VG_BOOL_CLO(arg, "--vprec-daz", bool_val)) {
    vr.vprec_conf.daz = bool_val;
  } else if (VG_BOOL_CLO(arg, "--vprec-ftz", bool_val)) {
    vr.vprec_conf.ftz = bool_val;
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
