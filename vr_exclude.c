
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to exclude some symbols     ---*/
/*--- from the instrumentation.                                    ---*/
/*---                                                 vr_exclude.c ---*/
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

#define LINE_SIZEMAX VR_FNNAME_BUFSIZE

#define UNAMED_FUNCTION_VERROU "unamed_function_verrou"
#define UNAMED_OBJECT_VERROU "unamed_object_verrou"

static Vr_Exclude* vr_addExclude (Vr_Exclude* list, const HChar * fnname, const HChar * objname) {
  Vr_Exclude * cell = VG_(malloc)("vr.addExclude.1", sizeof(Vr_Exclude));
  cell->fnname  = VG_(strdup)("vr.addExclude.2", fnname);
  cell->objname = VG_(strdup)("vr.addExclude.3", objname);
  cell->used    = False;
  cell->next    = list;
  return cell;
}

static Vr_Exclude *
vr_findExclude (Vr_Exclude* list, const HChar * fnname, const HChar * objname) {
  Vr_Exclude * exclude;
  for (exclude = list ; exclude != NULL ; exclude = exclude->next) {
    if (exclude->fnname[0] != '*'
	&& VG_(strcmp)(exclude->fnname, fnname) != 0)
      continue;

    if (exclude->objname[0] != '*'
	&& VG_(strcmp)(exclude->objname, objname) != 0)
      continue;

    return exclude;
  }

  return NULL;
}

void vr_freeExcludeList (Vr_Exclude* list) {
  while (list != NULL) {
    if(!list->used){
       VG_(umsg)("Warning exclude unused: %s\t%s\n", list->fnname,  list->objname );
    }
    Vr_Exclude *next = list->next;
    VG_(free)(list->fnname);
    VG_(free)(list->objname);
    VG_(free)(list);
    list = next;
  }
}

void
vr_dumpExcludeList (Vr_Exclude* list, Vr_Exclude* end, const HChar* fname) {
  Int fd = VG_(fd_open)(fname,
			VKI_O_CREAT|VKI_O_TRUNC|VKI_O_WRONLY,
			VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IWGRP);
  VG_(umsg)("Dumping exclusions list to `%s'... ", fname);
  if (fd == -1) {
    VG_(umsg)("ERROR!\n");
    return;
  }

  Vr_Exclude * exclude;
  for (exclude = list ; exclude != end ; exclude = exclude->next) {
    VG_(write)(fd, exclude->fnname, VG_(strlen)(exclude->fnname));
    VG_(write)(fd, "\t", 1);
    VG_(write)(fd, exclude->objname, VG_(strlen)(exclude->objname));
    VG_(write)(fd, "\n", 1);
  }
  VG_(close)(fd);

  VG_(umsg)("OK.\n");
}

Vr_Exclude * vr_loadExcludeList (Vr_Exclude * list, const HChar * fname) {
  VG_(umsg)("Loading exclusions list from `%s'... ", fname);
  Int fd = VG_(fd_open)(fname,VKI_O_RDONLY, 0);
  if (fd == -1) {
    VG_(umsg)("ERROR (open)\n");
    return list;
  }

  SizeT nLine = LINE_SIZEMAX;
  HChar *line = VG_(malloc)("vr.loadExcludes.1", nLine*sizeof(HChar));
  Int lineno = 0;

  while (! VG_(get_line (fd, &line, &nLine, &lineno))) {
    if( *line== '#'){ //Workaround : VG_(get_line) can return a comment line for the last line without \n
       continue;
    }
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

    list = vr_addExclude (list,
			  line, /*fnname=*/
			  c);/*objname*/
  }

  VG_(free)(line);
  VG_(close)(fd);

  VG_(umsg)("OK.\n");

  return list;
}

Bool vr_excludeIRSB (const HChar** fnnamePtr, const HChar **objnamePtr) {
  // Never exclude anything when generating the list
  if (vr.genExclude)
    return False;

  // Never exclude functions / objects unless they are explicitly listed
  Vr_Exclude *exclude = vr_findExclude (vr.exclude, *fnnamePtr, *objnamePtr);
  if (exclude == NULL) {
    return False;
  }



  // Inform the first time a rule is used
  if (!exclude->used) {
    VG_(umsg)("Using exclusion rule: %s\t%s\n", exclude->fnname, exclude->objname);
    exclude->used = True;
  }

  return True;
}


void
vr_excludeIRSB_generate (const HChar** fnnamePtr, const HChar **objnamePtr) {

  // Never exclude functions / objects unless they are explicitly listed
  Vr_Exclude *exclude = vr_findExclude (vr.exclude, *fnnamePtr, *objnamePtr);
  if(exclude==NULL){
    vr.exclude = vr_addExclude (vr.exclude, *fnnamePtr, *objnamePtr);
  }
}



Vr_IncludeSource*
vr_addIncludeSource (Vr_IncludeSource* list, const HChar* fnname,
		     const HChar * filename, UInt linenum) {
  Vr_IncludeSource * cell = VG_(malloc)("vr.addIncludeSource.1", sizeof(Vr_IncludeSource));
  cell->fnname   = VG_(strdup)("vr.addIncludeSource.2", fnname);
  cell->filename = VG_(strdup)("vr.addIncludeSource.3", filename);
  cell->linenum  = linenum;
  cell->next     = list;
  return cell;
}

static Vr_IncludeSource *
vr_findIncludeSource (Vr_IncludeSource* list,
		      const HChar* fnname,
		      const HChar * filename, UInt linenum) {
  Vr_IncludeSource * cell;
  for (cell = list ; cell != NULL ; cell = cell->next) {
    if (cell->linenum != linenum)
      continue;

    if (VG_(strcmp)(cell->filename, filename) != 0)
      continue;

    if (VG_(strcmp)(cell->fnname, fnname) != 0)
      continue;

    return cell;
  }

  return NULL;
}

void vr_freeIncludeSourceList (Vr_IncludeSource* list) {
  while (list != NULL) {
    Vr_IncludeSource *next = list->next;
    VG_(free)(list->fnname);
    VG_(free)(list->filename);
    VG_(free)(list);
    list = next;
  }
}

void vr_dumpIncludeSourceList (Vr_IncludeSource * list, Vr_IncludeSource* end,
			       const HChar * fname) {
  Int fd = VG_(fd_open)(fname,
			VKI_O_CREAT|VKI_O_TRUNC|VKI_O_WRONLY,
			VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IWGRP);
  VG_(umsg)("Dumping list of included sources to `%s'... ", fname);
  if (fd == -1) {
    VG_(umsg)("ERROR!\n");
    return;
  }

  HChar linenum_[256];
  Vr_IncludeSource * cell;
  for (cell = list ; cell != end ; cell = cell->next) {
    VG_(write)(fd, cell->filename, VG_(strlen)(cell->filename));
    VG_(write)(fd, "\t", 1);
    VG_(snprintf)(linenum_, 255, "%u", cell->linenum);
    VG_(write)(fd, linenum_, VG_(strlen)(linenum_));
    VG_(write)(fd, "\t", 1);
    VG_(write)(fd, cell->fnname, VG_(strlen)(cell->fnname));
    VG_(write)(fd, "\n", 1);
  }
  VG_(close)(fd);

  VG_(umsg)("OK.\n");
}

Vr_IncludeSource *
vr_loadIncludeSourceList (Vr_IncludeSource * list, const HChar * fname) {
  VG_(umsg)("Loading list of included sources from `%s'... ", fname);
  Int fd = VG_(fd_open)(fname,VKI_O_RDONLY, 0);
  if (fd == -1) {
    VG_(umsg)("ERROR (open)\n");
    return list;
  }

  SizeT nLine = LINE_SIZEMAX ;
  HChar *line = VG_(malloc)("vr.loadIncludeSources.1", nLine*sizeof(HChar));
  Int lineno = 0;

  while (! VG_(get_line (fd, &line, &nLine, &lineno))) {
    if( *line== '#'){//Workaround : VG_(get_line) can return a comment line for the last line without \n
       continue;
    }
    HChar * c;
    HChar* filename = line;
    // Skip non-blank characters
    for (c = line;
	 c<line+LINE_SIZEMAX && *c != 0 && *c != '\t' && *c != ' ';
	 ++c) {}
    if (*c == 0 || c>line+LINE_SIZEMAX-1) {
      VG_(umsg)("ERROR (parse1) : %s\n",line);
      return list;
    }
    *c = 0;

    // Skip blank characters
    for (++c;
	 c<line+LINE_SIZEMAX && *c != 0 && (*c == '\t' || *c == ' ');
	 ++c) {}
    HChar* linenum_ = c;
    // Skip non-blank characters
    for (;
	 c<line+LINE_SIZEMAX && *c != 0 && *c != '\t' && *c != ' ';
	 ++c) {}
    if (c>line+LINE_SIZEMAX-1) {
      VG_(umsg)("ERROR (parse2) : %s\n",line);
      return list;
    }
    if (*c==0) {
      c = line + LINE_SIZEMAX;
    } else {
      *c = 0;
      ++c;
    }
    UInt linenum = VG_(strtoull10)(linenum_,NULL);

    // Skip blank characters
    for (;
	 c<line+LINE_SIZEMAX && *c != 0 && (*c == '\t' || *c == ' ');
	 ++c) {}
    HChar* fnname = c;

    list = vr_addIncludeSource (list,fnname,filename,linenum);
  }

  VG_(free)(line);
  VG_(close)(fd);

  VG_(umsg)("OK.\n");

  return list;
}

void
vr_includeSource_generate (Vr_IncludeSource** list,
			   const HChar* fnname,
			   const HChar* filename, UInt linenum){
  if (vr_findIncludeSource(*list, fnname, filename, linenum) == NULL) {
    *list = vr_addIncludeSource (*list, fnname, filename, linenum);
  }
}

Bool
vr_includeSource (Vr_IncludeSource** list,
		  const HChar* fnname, const HChar* filename, UInt linenum) {
  return vr_findIncludeSource(*list, fnname, filename, linenum) != NULL;
}
