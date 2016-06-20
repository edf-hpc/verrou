
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

void vr_env_clo (const HChar* env, const HChar *clo) {
  HChar* val = VG_(getenv)(env);
  if (val) {
    HChar tmp[256];
    VG_(snprintf)(tmp, 255, "%s=%s", clo, val);
    vr_process_clo(tmp);
  }
}

void vr_clo_defaults (void) {
  vr.roundingMode = VR_NEAREST;
  vr.count = True;
  vr.instr_scalar = False;
  vr.instrument = VR_INSTR_ON;
  vr.verbose = False;

  vr.genExclude = False;
  vr.exclude = NULL;

  vr.genIncludeSource = False;
  vr.includeSource = NULL;

  int opIt;
  for(opIt=0 ; opIt<VR_OP ; opIt++){
    vr.instr_op[opIt]=False;
  }
}

Bool vr_process_clo (const HChar *arg) {
  Bool bool_val;
  const HChar * str;

  //Option --rounding-mode=
  if      (VG_XACT_CLO (arg, "--rounding-mode=random",
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

  //Options to choose op to instrument
  else if (VG_BOOL_CLO (arg, "--vr-instr-scalar", bool_val)) {
    vr.instr_scalar= bool_val;
  }

  //Option --verrou-verbose (to avoid verbose of valgrind)
  else if (VG_BOOL_CLO (arg, "--verrou-verbose", bool_val)) {
    vr.verbose = bool_val;
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
    vr.excludeFile = VG_(strdup)("vr.process_clo.1", str);
    vr.genExclude = True;
  }
  else if (VG_STR_CLO (arg, "--exclude", str)) {
    vr.exclude = vr_loadExcludeList(vr.exclude, str);
  }

  // Instrumentation of only specified source lines
  else if (VG_STR_CLO (arg, "--gen-source", str)) {
    vr.includeSourceFile = VG_(strdup)("vr.process_clo.2", str);
    vr.genIncludeSource = True;
  }
  else if (VG_STR_CLO (arg, "--source", str)) {
    vr.includeSource = vr_loadIncludeSourceList(vr.includeSource, str);
  }

  // Unknown option
  else{
    return False;
  }

  return True;
}

void vr_print_usage (void) {
  VG_(printf)
    ("\n"
     "    Rounding mode selection \n"
     "      --rounding-mode=random\n"
     "      --rounding-mode=average\n"
     "      --rounding-mode=nearest\n"
     "      --rounding-mode=upward\n"
     "      --rounding-mode=downward\n"
     "      --rounding-mode=toward_zero\n"
     "\n"
     "    Instrumented operations selection \n"
     "      --vr-instr=add\n"
     "      --vr-instr=sub\n"
     "      --vr-instr=mul\n"
     "      --vr-instr=div\n"
     "      --vr-instr=mAdd\n"
     "      --vr-instr=mSub\n"
     "\n"
     "    Restriction of instrumentation to specific code sections \n"
     "      --exclude=FILE      symbols listed in FILE will be left uninstrumented\n"
     "      --gen-exclude=FILE  generate in FILE a list of all encountered symbols\n"
     "                            (--gen-exclude and --exclude are mutually exclusive)\n"
     "      --source=FILE       when this option is present, only instructions coming\n"
     "                            from source code lines listed in FILE are\n"
     "                            instrumented.\n"
     "      --gen-source=FILE   generate in FILE a list of all source code lines\n"
     "                            encountered during program execution. This is in\n"
     "                            combination with the --exclude switch.\n"
     "                            (--source and --gen-source are mutually exclusive)\n"
     "\n"
     "    Other options \n"
     "      --vr-instr-scalar=[yes|NO] instrument scalar operation (x387)\n"
     "      --verrou-verbose=[yes|NO]  print each instrumentation switch\n"
     "      --count-op=[YES|no]        count floating point operations\n"
     "      --instr-atstart=[YES|no]   instrumentation from the start (useful when\n"
     "                                   used with client requests)\n"
     );
}

void vr_print_debug_usage (void) {
  vr_print_usage();
}
