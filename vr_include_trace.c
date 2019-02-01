
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to exclude some symbols     ---*/
/*--- from the instrumentation.                                    ---*/
/*---                                                 vr_exclude.c ---*/
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

#define LINE_SIZEMAX VR_FNNAME_BUFSIZE

static Vr_Include_Trace* vr_addIncludeTrace (Vr_Include_Trace* list, const HChar * fnname, const HChar * objname) {
  Vr_Include_Trace * cell = VG_(malloc)("vr.addIncludeTrace.1", sizeof(Vr_Include_Trace));
  cell->fnname  = VG_(strdup)("vr.addIncludeTrace.2", fnname);
  cell->objname = VG_(strdup)("vr.addIncludeTrace.3", objname);
  cell->next    = list;
  return cell;
}

static Vr_Include_Trace * vr_findIncludeTrace (Vr_Include_Trace* list, const HChar * fnname, const HChar * objname) {
  Vr_Include_Trace * include;
  for (include = list ; include != NULL ; include = include->next) {
    if (include->fnname[0] != '*'
        && VG_(strcmp)(include->fnname, fnname) != 0)
      continue;

    if (include->objname[0] != '*'
        && VG_(strcmp)(include->objname, objname) != 0)
      continue;

    return include;
  }

  return NULL;
}

void vr_freeIncludeTraceList (Vr_Include_Trace* list) {
  while (list != NULL) {
    Vr_Include_Trace *next = list->next;
    VG_(free)(list->fnname);
    VG_(free)(list->objname);
    VG_(free)(list);
    list = next;
  }
}


Vr_Include_Trace * vr_loadIncludeTraceList (Vr_Include_Trace * list, const HChar * fname) {
  VG_(umsg)("Loading inclusion trace list from `%s'... ", fname);
  Int fd = VG_(fd_open)(fname,VKI_O_RDONLY, 0);
  if (fd == -1) {
    VG_(umsg)("ERROR (open)\n");
    return list;
  }

  SizeT nLine = LINE_SIZEMAX;
  HChar *line = VG_(malloc)("vr.loadIncludeTrace.1", nLine*sizeof(HChar));
  Int lineno = 0;

  while (! VG_(get_line)(fd, &line, &nLine, &lineno)) {
    HChar * c;

    // Skip non-blank characters
    for (c = line;
         c<line+LINE_SIZEMAX && *c != 0 && *c != '\t' && *c != ' ';
         ++c) {}
    if (*c == 0 || c>line+LINE_SIZEMAX-1) {
      VG_(umsg)("ERROR (parse) :%s \n",line);
      return list;
    }
    *c = 0;

    // Skip blank characters
    for (++c;
         c<line+LINE_SIZEMAX && *c != 0 && (*c == '\t' || *c == ' ');
         ++c) {}
    list = vr_addIncludeTrace (list,
			       /*fnname=*/ line,
			       /*objname*/ c);;
  }
  VG_(free)(line);
  VG_(close)(fd);
  VG_(umsg)("OK.\n");
  return list;
}



Bool vr_includeTraceIRSB (const HChar** fnname, const HChar **objname) {
  if (** fnname == 0) {
    return False;
  }
  if (** objname == 0) {
    return False;
  }
  // Never exclude functions / objects unless they are explicitly listed
  Vr_Include_Trace *include = vr_findIncludeTrace (vr.includeTrace, *fnname, *objname);
  if (include != NULL) {
    return True;
  }
  return False;
}
