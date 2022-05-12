
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
#include "pub_tool_transtab.h"       // VG_(discard_translations_safely)
// * Start-stop instrumentation

#ifdef PROFILING_EXACT
#include "interflop_backends/interflop_verrou/interflop_verrou.h"
#endif

void vr_set_instrument_state (const HChar* reason, Vr_Instr state, Bool discard) {
  if (vr.instrument == state) {
    if(vr.verbose){
      VG_(message)(Vg_DebugMsg,"%s: instrumentation already %s\n",
		   reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
    }

    return;
  }

  vr.instrument = state;

  if(discard){
     VG_(discard_translations_safely)( (Addr)0x1000, ~(SizeT)0xfff, "verrou");
  }
     /* if(vr.instrument == VR_INSTR_ON){ */
  /*   verrou_begin_instr(); */
  /* }else{ */
  /*   verrou_end_instr(); */
  /* } */

  if(vr.verbose){
    VG_(message)(Vg_DebugMsg, "%s: instrumentation switched %s\n",
		 reason, (state==VR_INSTR_ON) ? "ON" : "OFF");
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
  verrou_set_seed (hash);
#ifdef USE_VERROU_QUAD
  mcaquad_set_seed (hash);
#endif
  VG_(message)(Vg_DebugMsg, "Entering deterministic section %u: %s\n",
               hash, name);
}

static void vr_stop_deterministic_section (unsigned int level) {
  HChar name[256];
  vr_deterministic_section_name (level, name, 256);

  VG_(message)(Vg_DebugMsg, "Leaving deterministic section: %s\n",
               name);
  verrou_set_random_seed ();
#ifdef USE_VERROU_QUAD 
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

// * Handle client requests

// ** GDB monitor commands

static void vr_handle_monitor_instrumentation_print (void) {
  VG_(gdb_printf) ("instrumentation: %s\n",
                   vr.instrument==VR_INSTR_ON ? "on" : "off");
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
     vr_set_instrument_state("Monitor", VR_INSTR_ON, True);
    vr_handle_monitor_instrumentation_print();
    return True;
  case 1: /* off */
     vr_set_instrument_state("Monitor", VR_INSTR_OFF, True);
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
  }
  return True;
}
