
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code related to client requests handling. ---*/
/*---                                                   vr_clreq.c ---*/
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
#include "vr_clreq.h"

#include "pub_tool_transtab.h"       // VG_(discard_translations_safely)
// * Start-stop instrumentation

//#ifdef PROFILING_EXACT
#include "interflop_backends/interflop_verrou/interflop_verrou.h"
//#endif
#include "interflop_backends/backend_checkdenorm/interflop_checkdenorm.h"

void vr_set_instrument_state (const HChar* reason, Vr_Instr state, Bool isHard) {
   if(isHard){
      if (vr.instrument_hard == state) {
         if(vr.verbose){
            VG_(message)(Vg_DebugMsg,"%s: instrumentation already %s\n",
                         reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
         }
         return;
      }

      vr.instrument_hard = state;

      VG_(discard_translations_safely)( (Addr)0x1000, ~(SizeT)0xfff, "verrou");
      vr_clean_cache();

      if(vr.instrument_soft_used){
         if(vr.instrument_hard && vr.instrument_soft){
            vr.instrument_soft_used=False;
         }
      }

      if(vr.verbose){
         VG_(message)(Vg_DebugMsg, "%s: instrumentation switched %s\n",
                      reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
      }
   }else{//soft
      if(vr.instrument_soft_used==False && vr.instrument_hard==VR_INSTR_ON ){
         VG_(discard_translations_safely)( (Addr)0x1000, ~(SizeT)0xfff, "verrou");
      }

      vr.instrument_soft_used= True;
      if (vr.instrument_soft == state) {
         if(vr.verbose){
            VG_(message)(Vg_DebugMsg,"%s: instrumentation (soft) already %s\n",
                         reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
         }
         return;
      }
      vr.instrument_soft = state;
      vr_clean_cache();

      if(vr.verbose){
         VG_(message)(Vg_DebugMsg, "%s: instrumentation (soft) switched %s\n",
                      reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
      }
   }
}


void vr_set_rounding_mode(const char* modeStr){
   enum vr_RoundingMode mode= verrou_rounding_mode_enum(modeStr);

   if(mode==VR_ENUM_SIZE){
      VG_(umsg)("Invalid mode %s\n", modeStr);
   }else{
      if(mode!=vr.roundingMode){
         vr.roundingMode=mode;
         verrou_set_rounding_mode(mode);
         VG_(discard_translations_safely)( (Addr)0x1000, ~(SizeT)0xfff, "verrou");
         if(vr.verbose){
            VG_(umsg)("New rounding mode: ", modeStr);
         }
      }
   }
}

// * Enter/leave deterministic section

static void vr_deterministic_section_name (unsigned int level,
                                           HChar * name,
                                           unsigned int len)
{
  Addr ips[8];
  const HChar* fnname;
  const HChar* filename;
  UInt  linenum;
  Addr  addr;
  DiEpoch de = VG_(current_DiEpoch)();

  VG_(get_StackTrace)(VG_(get_running_tid)(),
                      ips, 8,
                      NULL, NULL,
                      0);
  addr = ips[level];

  //fnname[0] = 0;
  VG_(get_fnname)(de, addr, &fnname);

  //  filename[0] = 0;
  VG_(get_filename_linenum)(de,
                            addr,
                            &filename,
                            NULL,
                            &linenum);
  VG_(snprintf)(name, len,
                "%s (%s:%u)", fnname, filename, linenum);
}

static ULong vr_deterministic_section_hash (HChar const*const name)
{
  ULong hash = VG_(getpid)();
  int i = 0;
  while (name[i] != 0) {
    hash += i * name[i];
    ++i;
  }
  return hash;
}

static void vr_start_deterministic_section (unsigned int level) {
  HChar name[256];
  ULong hash;

  vr_deterministic_section_name (level, name, 256);

  hash = vr_deterministic_section_hash (name);
  verrou_seed_save_state ();
  verrou_set_seed (hash);
#ifdef USE_VERROU_QUADMATH
  mcaquad_set_seed (hash);
#endif
  VG_(message)(Vg_DebugMsg, "Entering deterministic section %llu: %s\n",
               hash, name);
}

static void vr_stop_deterministic_section (unsigned int level) {
  HChar name[256];
  vr_deterministic_section_name (level, name, 256);

  VG_(message)(Vg_DebugMsg, "Leaving deterministic section: %s\n",
               name);
  verrou_seed_restore_state ();
#ifdef USE_VERROU_QUADMATH
  mcaquad_set_random_seed ();
#endif
}

#ifdef PROFILING_EXACT
static void vr_print_profiling_exact(void){
  unsigned int num;
  unsigned int numExact;
  verrou_get_profiling_exact(&num,&numExact);
  VG_(umsg)("%s #total: %u #exact: %u\n","Profiling exact", num, numExact);
  }
#endif

void vr_print_denorm_counter(void){
   if(vr.backend==vr_checkdenorm){
      checkdenorm_print_counter(&VG_(umsg));
   }else{
      if(vr.verbose){
         VG_(umsg)("PRINT_COUNT_DENORM client request needs --count-denorm=yes\n");
      }
   }
}
void vr_reset_denorm_counter(void){
   if(vr.backend==vr_checkdenorm){
      checkdenorm_reset_counter();
   }else{
      if(vr.verbose){
         VG_(umsg)("RESET_COUNT_DENORM client request needs --count-denorm=yes\n");
      }
   }
}


// * Handle client requests

// ** GDB monitor commands

static void vr_handle_monitor_instrumentation_print (void) {
  VG_(gdb_printf) ("instrumentation: %s\n",
                   (vr.instrument_hard && vr.instrument_soft) ==VR_INSTR_ON ? "on" : "off");
}

static Bool vr_handle_monitor_instrumentation (HChar ** ssaveptr) {
  HChar * arg = VG_(strtok_r)(0, " ", ssaveptr);

  if (!arg) { /* no argument */
    vr_handle_monitor_instrumentation_print();
    return True;
  }

  switch (VG_(keyword_id) ("on off", arg, kwd_report_duplicated_matches)) {
  case -2: /* multiple matches */
    return True;
  case -1: /* not found */
    return False;
  case 0: /* on */
     vr_set_instrument_state("Monitor", VR_INSTR_ON, False);
     vr_handle_monitor_instrumentation_print();
     return True;
  case 1: /* off */
     vr_set_instrument_state("Monitor", VR_INSTR_OFF, False);
     vr_handle_monitor_instrumentation_print();
     return True;
  }
  return False;
}

static Bool vr_handle_monitor_help (void) {
  VG_(gdb_printf)("\n");
  VG_(gdb_printf)("verrou monitor commands:\n");
  VG_(gdb_printf)("  help                     : print this help\n");
  VG_(gdb_printf)("  count                    : print instruction counters\n");
  VG_(gdb_printf)("  instrumentation          : get instrumentation state\n");
  VG_(gdb_printf)("  instrumentation [on|off] : set instrumentation state\n");
  VG_(gdb_printf)("\n");
  return True;
}

static Bool vr_handle_monitor_command (HChar * req) {
    HChar * wcmd;
    HChar s[VG_(strlen(req)) + 1];
    HChar *ssaveptr;

    VG_(strcpy)(s, req);

    wcmd = VG_(strtok_r)(s, " ", &ssaveptr);
    switch (VG_(keyword_id) ("help instrumentation count",
                             wcmd, kwd_report_duplicated_matches)) {
    case -2: /* multiple matches */
      return True;
    case -1: /* not found */
      return False;
    case 0: /* help */
      return vr_handle_monitor_help ();
    case 1: /* instrumentation */
      return vr_handle_monitor_instrumentation (&ssaveptr);
    case 2: /* count */
      vr_ppOpCount();
      return True;
    }
    return False;
}

static Bool vr_isInstrumentedClr(Vr_Prec prec){
   Bool res=(vr.instrument_hard && vr.instrument_soft) && vr.instr_prec[prec];
   if((vr.genBackTraceBool || vr.useBackTraceBool) && res){
      vr_backtrace_dyn_BB();
   }
   return res && vr.instrument_soft_back;
}
// ** Client requests entry point

Bool vr_handle_client_request (ThreadId tid, UWord *args, UWord *ret) {
  if (!VG_IS_TOOL_USERREQ('V','R', args[0])
      && VG_USERREQ__GDB_MONITOR_COMMAND != args[0])
    return False;

  switch (args[0]) {
  case VR_USERREQ__START_INSTRUMENTATION:
     vr_set_instrument_state ("Client Request", True, True);
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__STOP_INSTRUMENTATION:
     vr_set_instrument_state ("Client Request", False, True);
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__START_SOFT_INSTRUMENTATION:
     vr_set_instrument_state ("Client Request", True, False);
     *ret = 0; /* meaningless */
     break;
  case VR_USERREQ__STOP_SOFT_INSTRUMENTATION:
     vr_set_instrument_state ("Client Request", False, False);
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
  case VR_USERREQ__DISPLAY_COUNTERS:
    vr_ppOpCount();
    *ret = 0; /* meaningless */
    break;
  case VR_USERREQ__DUMP_COVER:
    *ret=vr_traceBB_dumpCov();
    break;
  case VR_USERREQ__PRINT_PROFILING_EXACT:
#ifdef PROFILING_EXACT
    vr_print_profiling_exact();
    verrou_init_profiling_exact();
#else
    VG_(umsg)("Warning : verrou need to be compiled with --enable-verrou-profiling-exact to use the client request VERROU_PRINT_PROFILING_EXACT;\n");
#endif
      *ret=0;
      break;
  case VR_USERREQ__COUNT_FP_INSTRUMENTED:
     *ret=(UWord)vr_count_fp_instrumented();
     break;
  case VR_USERREQ__COUNT_FP_NOT_INSTRUMENTED:
     *ret=(UWord)vr_count_fp_not_instrumented();
     break;
  case VG_USERREQ__GDB_MONITOR_COMMAND:
    if (vr_handle_monitor_command((HChar*)args[1])) {
      *ret = 1;
      return True;
    } else {
      *ret = 0;
      return False;
    }
    break;
  case VR_USERREQ__SET_SEED:
    vr.firstSeed=args[1];
    VG_(umsg)("New seed : %llu\n", vr.firstSeed);
    verrou_set_seed (vr.firstSeed);
    vr_clean_cache_seed();
#ifdef USE_VERROU_QUADMATH
    mcaquad_set_seed(vr.firstSeed);
#endif
    *ret = 0; /* meaningless */
    break;

  case VR_USERREQ__PRANDOM_UPDATE:
    verrou_updatep_prandom();
    if(vr.verbose){
      if(   (vr.roundingMode==VR_PRANDOM)||(vr.roundingMode==VR_PRANDOM_DET) ||(vr.roundingMode==VR_PRANDOM_COMDET)){
	VG_(umsg)("\tPRANDOM_UPDATE: pvalue=%f\n", verrou_prandom_pvalue());
      }
    }
    break;
  case VR_USERREQ__PRANDOM_UPDATE_VALUE:
    {
      double value=VG_(strtod)((char*)args[1],NULL);
      verrou_updatep_prandom_double(value);
    }
    if(vr.verbose){
      if(   (vr.roundingMode==VR_PRANDOM)||(vr.roundingMode==VR_PRANDOM_DET) ||(vr.roundingMode==VR_PRANDOM_COMDET)){
	VG_(umsg)("\tPRANDOM_UPDATE_VALUE: pvalue=%f\n", verrou_prandom_pvalue());
      }
    }
    break;
  case VR_USERREQ__GET_ROUNDING:
     *ret=(UWord)vr.roundingMode;
     break;
  case VR_USERREQ__GET_LIBM_ROUNDING:
     *ret=(UWord)vr.roundingMode;
     break;
  case VR_USERREQ__GET_LIBM_ROUNDING_NO_INST:
     *ret=(UWord)vr.roundingModeNoInst;
     break;

  case VR_USERREQ__NAN_DETECTED:
     vr_handle_NaN();
     break;

  case VR_USERREQ__INF_DETECTED:
     vr_handle_Inf();
     break;
  case VR_USERREQ__IS_INSTRUMENTED_FLOAT:
     *ret=(UWord)(vr_isInstrumentedClr(VR_PREC_FLT));
    break;
  case VR_USERREQ__IS_INSTRUMENTED_DOUBLE:
    *ret=(UWord)(vr_isInstrumentedClr(VR_PREC_DBL));
    break;
  case VR_USERREQ__IS_INSTRUMENTED_LDOUBLE:
    *ret=(UWord)(vr_isInstrumentedClr(VR_PREC_LDBL));
    break;
  case VR_USERREQ__COUNT_OP:
    *ret=(UWord)( (Bool)(vr.count) );
    break;
  case VR_USERREQ__GENERATE_EXCLUDE_SOURCE:
    vr_generate_exclude_source((const char*)args[1], *(int*)args[2], (const char*)args[3], (const char*)args[4] );
     break;
  case VR_USERREQ__IS_INSTRUMENTED_EXCLUDE_SOURCE:
    *ret=(UWord)( (Bool)(vr_clrIsInstrumented((const char*)args[1], *(int*)args[2], (const char*)args[3],(const char*)args[4] )));
     break;
  case VR_USERREQ__REGISTER_CACHE:
     vr_register_cache((unsigned int*) args[1], (unsigned int)args[2]);
     break;
  case VR_USERREQ__GET_SEED:
    *ret = verrou_get_seed();
    break;
  case VR_USERREQ__REGISTER_CACHE_SEED:
     vr_register_cache_seed((unsigned int*) args[1]);
     break;
  case VR_USERREQ__FLOAT_CONV:
    *ret=(UWord)( vr.float_conv );
    break;
  case VR_USERREQ__PRINT_COUNT_DENORM:
     vr_print_denorm_counter();
     break;
  case VR_USERREQ__RESET_COUNT_DENORM:
     vr_reset_denorm_counter();
     break;
  case VR_USERREQ__SET_ROUNDING_MODE:
     vr_set_rounding_mode((char const*const)args[1]);
     break;
  }
  return True;
}
