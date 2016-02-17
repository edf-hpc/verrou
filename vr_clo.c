#include "vr_main.h"

void vr_clo_defaults (void) {
  vr.roundingMode = VR_NEAREST;
  vr.genExclude = False;
  vr.count = True;
  vr.instr_scalar = False;
  vr.instrument = VR_INSTR_ON;
  vr.verbose = False;

  int opIt;
  for(opIt=0; opIt< VR_OP;opIt++){
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

  // Exclusion files
  else if (VG_STR_CLO (arg, "--gen-exclude", str)) {
    vr.excludeFile = VG_(strdup)("vr.process_clo.1", str);
    vr.genExclude = True;
  }
  else if (VG_STR_CLO (arg, "--exclude", str)) {
    vr.excludeFile = VG_(strdup)("vr.process_clo.2", str);
    vr.genExclude = False;
  }

  // Unknown option
  else{
    return False;
  }

  return True;
}

void vr_print_usage (void) {
  VG_(printf)("\n");
  VG_(printf)("    Rounding mode selection \n");
  VG_(printf)("        --rounding-mode=random\n");
  VG_(printf)("        --rounding-mode=average\n");
  VG_(printf)("        --rounding-mode=nearest\n");
  VG_(printf)("        --rounding-mode=upward\n");
  VG_(printf)("        --rounding-mode=downward\n");
  VG_(printf)("        --rounding-mode=toward_zero\n");

  VG_(printf)("\n");
  VG_(printf)("    Instrumented operations selection \n");
  VG_(printf)("        --vr-instr=add\n");
  VG_(printf)("        --vr-instr=sub\n");
  VG_(printf)("        --vr-instr=mul\n");
  VG_(printf)("        --vr-instr=div\n");
  VG_(printf)("        --vr-instr=mAdd\n");
  VG_(printf)("        --vr-instr=mSub\n");

  VG_(printf)("\n");
  VG_(printf)("    Other options \n");
  VG_(printf)("        --vr-instr-scalar=[yes|NO] instrument scalar operation (x387)\n");
  VG_(printf)("        --verrou-verbose=[yes|NO]  print each instrumentation switch\n");
  VG_(printf)("        --count-op=[yes|NO]        count floating point operations\n");
  VG_(printf)("        --instr-atstart=[YES|no]   instrumentation from the start (useful when\n"
              "                                     used with client requests)\n");
  VG_(printf)("        --gen-exclude=FILE         generate excluded functions list in FILE\n");
  VG_(printf)("        --exclude=FILE             read excluded functions list from FILE\n");
  VG_(printf)("                                     (--gen-exclude and --exclude are\n"
              "                                     mutually exclusive)\n");
}

void vr_print_debug_usage (void) {
  vr_print_usage();
}
