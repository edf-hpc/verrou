
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code related errors handling.             ---*/
/*---                                                   vr_error.c ---*/
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
#include "coregrind/pub_core_debuginfo.h"

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
  case VR_ERROR_NAN:
    return "NaN";
  case VR_ERROR_CC:
    return "Cancellation";
  case VR_ERROR_CD:
    return "Denorm";
  case VR_ERROR_FLT_MAX:
    return "FLT_MAX";

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
  VG_(snprintf)(string, 10, "%u", op);

  Vr_Error extra;
  extra.instr.op = op;
  VG_(maybe_record_error)(tid,
                          kind,
                          addr,
                          string,
                          &extra);
}

static void vr_pp_ErrorOp (const Error* err) {
  Vr_Error *extra = VG_(get_error_extra)(err);

  VG_(umsg)("%s: ", vr_get_error_name(err));
  VG_(message_flush)();
  ppIROp (extra->instr.op);
  VG_(printf)(" (%s)", VG_(get_error_string)(err));
  VG_(umsg)("\n");
  VG_(pp_ExeContext)(VG_(get_error_where)(err));
}


// * Errors happening at run time

void vr_maybe_record_ErrorRt (Vr_ErrorKind kind) {
  ThreadId tid = VG_(get_running_tid)();
  Addr addr;
  VG_(get_StackTrace)(tid, &addr, 1, NULL, NULL, 0);

  HChar string[1];
  string[0] = 0;

  VG_(maybe_record_error)(tid,
                          kind,
                          addr,
                          string,
                          NULL);
}

void vr_handle_NaN () {
   if(vr.checknan){
      vr_maybe_record_ErrorRt(VR_ERROR_NAN);
   }
}

void vr_handle_FLT_MAX () {
   if(vr.checkFloatMax){
      vr_maybe_record_ErrorRt(VR_ERROR_FLT_MAX);
   }
}


void vr_handle_CC (int unused) {
   ThreadId tid = VG_(get_running_tid)();
   Addr addr;
   VG_(get_StackTrace)(tid, &addr, 1, NULL, NULL, 0);

   if(vr.dumpCancellation){
      DiEpoch di=VG_(current_DiEpoch)();
      const HChar* fileName;
      const HChar* dirName;
      const HChar* symName;
      UInt lineNum;
      //UInt errorName=
      VG_(get_filename_linenum)(di,addr,
                                &fileName,
                                &dirName,
                                &lineNum );
      VG_(get_fnname_raw)(di, addr, &symName);
//      VG_(umsg)("test ? %s - %s : %u   --> %u \n", symName,fileName, lineNum,errorName);
      vr_includeSource_generate (&vr.cancellationSource , symName, fileName, lineNum);
   }

   if(vr.checkCancellation){
      HChar string[1];
      string[0] = 0;
      VG_(maybe_record_error)(tid,
                              VR_ERROR_CC,
                              addr,
                              string,
                              NULL);
   }
}

void vr_handle_CD () {
   ThreadId tid = VG_(get_running_tid)();
   Addr addr;
   VG_(get_StackTrace)(tid, &addr, 1, NULL, NULL, 0);

   if(vr.dumpDenorm){
      DiEpoch di=VG_(current_DiEpoch)();
      const HChar* fileName;
      const HChar* dirName;
      const HChar* symName;
      UInt lineNum;
      //UInt errorName=
      VG_(get_filename_linenum)(di,addr,
                                &fileName,
                                &dirName,
                                &lineNum );
      VG_(get_fnname_raw)(di, addr, &symName);
//      VG_(umsg)("test ? %s - %s : %u   --> %u \n", symName,fileName, lineNum,errorName);
      vr_includeSource_generate (&vr.denormSource , symName, fileName, lineNum);
   }

   if(vr.checkDenorm){
      HChar string[1];
      string[0] = 0;
      VG_(maybe_record_error)(tid,
                              VR_ERROR_CD,
                              addr,
                              string,
                              NULL);
   }
}



static void vr_pp_ErrorRt (const Error* err) {
  VG_(umsg)("%s: ", vr_get_error_name(err));
  VG_(message_flush)();
  VG_(umsg)("\n");
  VG_(pp_ExeContext)(VG_(get_error_where)(err));
}


// * Standard tools interface

const HChar* vr_get_error_name (const Error* err) {
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

void vr_before_pp_Error (const Error* err) {}

void vr_pp_Error (const Error* err) {
  switch (VG_(get_error_kind)(err)) {
  case VR_ERROR_UNCOUNTED:
  case VR_ERROR_SCALAR:
    vr_pp_ErrorOp (err);
    break;
  case VR_ERROR_NAN:
  case VR_ERROR_CC:
  case VR_ERROR_CD:
  case VR_ERROR_FLT_MAX:
     vr_pp_ErrorRt (err);
     break;
  }
}

Bool vr_eq_Error (VgRes res, const Error* e1, const Error* e2) {
  return VG_(get_error_address)(e1) == VG_(get_error_address)(e2);
}

UInt vr_update_extra (const Error* err) {
  return sizeof(Vr_Error);
}

Bool vr_error_matches_suppression (const Error* err, const Supp* su) {
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

SizeT vr_print_extra_suppression_info (const Error* err,
                                      /*OUT*/HChar* buf, Int nBuf) {
  HChar* res=VG_(strncpy)(buf, VG_(get_error_string)(err), nBuf);
  SizeT len=VG_(strlen)(res);
  return len ;
}

SizeT vr_print_extra_suppression_use (const Supp* su,
                                     /*OUT*/HChar* buf, Int nBuf) {
  return (SizeT)0; //False
}

void vr_update_extra_suppression_use (const Error* err, const Supp* su) {}
