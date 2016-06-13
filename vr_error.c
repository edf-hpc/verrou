
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code related errors handling.             ---*/
/*---                                                   vr_error.c ---*/
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

typedef struct Vr_InstrError_ Vr_InstrError;
struct Vr_InstrError_ {
  IROp op;
};

typedef union Vr_Error_ Vr_Error;
union Vr_Error_ {
  Vr_InstrError instr;
};


static const HChar* vr_error_name (Vr_ErrorKind kind) {
  switch (kind) {
  case VR_ERROR_UNCOUNTED:
    return "Uncounted operation";
  case VR_ERROR_SCALAR:
    return "Scalar instruction";
  default:
    return NULL;
  }
}


// * Errors at the instruction level

void vr_maybe_record_ErrorOp (Vr_ErrorKind kind, IROp op) {
  ThreadId tid = VG_(get_running_tid)();
  Addr addr;
  VG_(get_StackTrace)(tid, &addr, 1, NULL, NULL, 0);

  HChar string[10];
  VG_(snprintf)(string, 10, "%d", op);

  Vr_Error extra;
  extra.instr.op = op;
  VG_(maybe_record_error)(tid,
                          kind,
                          addr,
                          string,
                          &extra);
}

static void vr_pp_ErrorOp (Error* err) {
  Vr_Error *extra = VG_(get_error_extra)(err);

  VG_(umsg)("%s: ", vr_get_error_name(err));
  VG_(message_flush)();
  ppIROp (extra->instr.op);
  VG_(printf)(" (%s)", VG_(get_error_string)(err));
  VG_(umsg)("\n");
  VG_(pp_ExeContext)(VG_(get_error_where)(err));
}


// * Standard tools interface

const HChar* vr_get_error_name (Error* err) {
  return vr_error_name (VG_(get_error_kind)(err));
}

Bool vr_recognised_suppression (const HChar* name, Supp* su) {
  Vr_ErrorKind kind;
  for (kind = 0 ; kind < VR_ERROR ; ++kind) {
    const HChar* errorName = vr_error_name(kind);
    if (errorName && VG_(strcmp)(name, errorName) == 0)
      break;
  }

  if (kind == VR_ERROR) {
    return False;
  } else {
    VG_(set_supp_kind)(su, 0);
    return True;
  }
}

void vr_before_pp_Error (Error* err) {}

void vr_pp_Error (Error* err) {
  switch (VG_(get_error_kind)(err)) {
  case VR_ERROR_UNCOUNTED:
  case VR_ERROR_SCALAR:
    vr_pp_ErrorOp (err);
    break;
  }
}

Bool vr_eq_Error (VgRes res, Error* e1, Error* e2) {
  return VG_(get_error_address)(e1) == VG_(get_error_address)(e2);
}

UInt vr_update_extra (Error* err) {
  return sizeof(Vr_Error);
}

Bool vr_error_matches_suppression (Error* err, Supp* su) {
  if (VG_(get_error_kind)(err) != VG_(get_supp_kind)(su)) {
    return False;
  }

  if (VG_(strcmp)(VG_(get_error_string)(err), VG_(get_supp_string)(su)) != 0) {
    return False;
  }

  return True;
}

Bool vr_read_extra_suppression_info (Int fd, HChar** bufpp, SizeT* nBufp,
                                     Int* lineno, Supp* su) {
  VG_(get_line)(fd, bufpp, nBufp, lineno);
  VG_(set_supp_string)(su, VG_(strdup)("vr.resi.1", *bufpp));
  return True;
}

Bool vr_print_extra_suppression_info (Error* err,
                                      /*OUT*/HChar* buf, Int nBuf) {
  VG_(strncpy)(buf, VG_(get_error_string)(err), nBuf);
  return True;
}

Bool vr_print_extra_suppression_use (Supp* su,
                                     /*OUT*/HChar* buf, Int nBuf) {
  return False;
}

void vr_update_extra_suppression_use (Error* err, Supp* su) {}
