
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code related command-line options.        ---*/
/*---                                                     vr_clo.c ---*/
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
//#include "backend_verrou/vr_rand.h"
#include "backend_verrou/interflop_verrou.h"
#include "backend_mcaquad/interflop_mcaquad.h"

void vr_env_clo (const HChar* env, const HChar *clo) {
  HChar* val = VG_(getenv)(env);
  if (val) {
    HChar tmp[256];
    VG_(snprintf)(tmp, 255, "%s=%s", clo, val);
    vr_process_clo(tmp);
  }
}

void vr_clo_defaults (void) {
  vr.backend = vr_verrou;
  vr.roundingMode = VR_NEAREST;
  vr.count = True;
  vr.instr_scalar = False;
  vr.instrument = VR_INSTR_ON;
  vr.verbose = False;
  vr.unsafe_llo_optim = False;

  vr.genExclude = False;
  vr.exclude = NULL;
  //  vr.genAbove = NULL;

  vr.genIncludeSource = False;
  vr.includeSource = NULL;

  int opIt;
  for(opIt=0 ; opIt<VR_OP ; opIt++){
    vr.instr_op[opIt]=False;
  }

  vr.firstSeed=(unsigned int)(-1);
  vr.mca_precision_double=53;
  vr.mca_precision_float=24;
  vr.mca_mode=MCAMODE_MCA;

  vr.checknan=True;

  vr.checkCancellation=False;
  vr.cc_threshold_float=18;
  vr.cc_threshold_double=40;

  vr.dumpCancellation=False;
  vr.cancellationSource=NULL;


}

Bool vr_process_clo (const HChar *arg) {
  Bool bool_val;
  const HChar * str;
  //Option --backend=
  if      (VG_XACT_CLO (arg, "--backend=verrou",
                        vr.backend, vr_verrou)) {}
  else if (VG_XACT_CLO (arg, "--backend=mcaquad",
                        vr.backend, vr_mcaquad)) {}

  //Option --rounding-mode=
  else if (VG_XACT_CLO (arg, "--rounding-mode=random",
                        vr.roundingMode, VR_RANDOM)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=average",
                        vr.roundingMode, VR_AVERAGE)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=nearest",
                        vr.roundingMode, VR_NEAREST)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=upward",
                        vr.roundingMode, VR_UPWARD)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=downward",
                        vr.roundingMode, VR_DOWNWARD)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=toward_zero",
                        vr.roundingMode, VR_ZERO)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=farthest",
                        vr.roundingMode, VR_FARTHEST)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=float",
                        vr.roundingMode, VR_FLOAT)) {}
  else if (VG_XACT_CLO (arg, "--rounding-mode=native",
                        vr.roundingMode, VR_NATIVE)) {}

  //Option mcaquad
  else if (VG_INT_CLO(arg, "--mca-precision-double",
                      vr.mca_precision_double)){}
  else if (VG_INT_CLO(arg, "--mca-precision-float",
                        vr.mca_precision_float)){}
  else if (VG_XACT_CLO (arg, "--mca-mode=rr",
                        vr.mca_mode, MCAMODE_RR)) {}
  else if (VG_XACT_CLO (arg, "--mca-mode=pb",
                        vr.mca_mode, MCAMODE_PB)) {}
  else if (VG_XACT_CLO (arg, "--mca-mode=mca",
                        vr.mca_mode, MCAMODE_MCA)) {}
  else if (VG_XACT_CLO (arg, "--mca-mode=ieee",
                        vr.mca_mode, MCAMODE_IEEE)) {}

  //Options to choose op to instrument
  else if (VG_XACT_CLO (arg, "--vr-instr=add",
                        vr.instr_op[VR_OP_ADD] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=sub",
                        vr.instr_op[VR_OP_SUB] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mul",
                        vr.instr_op[VR_OP_MUL] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=div",
                        vr.instr_op[VR_OP_DIV] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mAdd",
                        vr.instr_op[VR_OP_MADD] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=mSub",
                        vr.instr_op[VR_OP_MSUB] , True)) {}
  else if (VG_XACT_CLO (arg, "--vr-instr=conv",
                        vr.instr_op[VR_OP_CONV] , True)) {}

  //Option to enable check-cancellation backend
  else if (VG_BOOL_CLO (arg, "--check-cancellation", bool_val)) {
     vr.checkCancellation= bool_val;
  }
  else if (VG_INT_CLO(arg, "--cc-threshold-double",
                      vr.cc_threshold_double)){}
  else if (VG_INT_CLO(arg, "--cc-threshold-float",
                      vr.cc_threshold_float)){}

  else if (VG_BOOL_CLO (arg, "--check-nan", bool_val)) {
     vr.checknan= bool_val;
  }

  //Options to choose op to instrument
  else if (VG_BOOL_CLO (arg, "--vr-instr-scalar", bool_val)) {
    vr.instr_scalar= bool_val;
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
  else if (VG_BOOL_CLO (arg, "--instr-atstart", bool_val)) {
    vr.instrument = bool_val ? VR_INSTR_ON : VR_INSTR_OFF;
  }

  // Exclusion of specified symbols
  else if (VG_STR_CLO (arg, "--gen-exclude", str)) {
    //vr.excludeFile = VG_(strdup)("vr.process_clo.gen-exclude", str);
    vr.excludeFile = VG_(expand_file_name)("vr.process_clo.gen-exclude", str);
    vr.genExclude = True;
  }
  /* else if (VG_STR_CLO (arg, "--gen-above", str)) { */
  /*   vr.genAbove = VG_(strdup)("vr.process_clo.gen-above", str); */
  /* } */
  else if (VG_STR_CLO (arg, "--exclude", str)) {
    vr.exclude = vr_loadExcludeList(vr.exclude, str);
  }

  // Instrumentation of only specified source lines
  else if (VG_STR_CLO (arg, "--gen-source", str)) {
    //vr.includeSourceFile = VG_(strdup)("vr.process_clo.gen-source", str);
    vr.includeSourceFile = VG_(expand_file_name)("vr.process_clo.gen-source", str);
    vr.genIncludeSource = True;
  }
  else if (VG_STR_CLO (arg, "--source", str)) {
    vr.includeSource = vr_loadIncludeSourceList(vr.includeSource, str);
  }

  else if (VG_STR_CLO (arg, "--cc-gen-file", str)) {
     vr.cancellationDumpFile  = VG_(expand_file_name)("vr.process_clo.cc-file", str);
     vr.dumpCancellation=True;
  }
  // Set the pseudo-Random Number Generator
  else if (VG_STR_CLO (arg, "--vr-seed", str)) {
    //vr_rand_setSeed (&vr_rand, VG_(strtoull10)(str, NULL));
    vr.firstSeed=VG_(strtoull10)(str, NULL);
    if(vr.firstSeed==(unsigned int)(-1)){
      VG_(tool_panic) ( "--vr-seed=-1 no taken into account\n");
    }
  }

  // Unknown option
  else{
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
