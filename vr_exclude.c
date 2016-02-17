#include "vr_main.h"

Vr_Exclude* vr_addExclude (Vr_Exclude* list, HChar * fnname, HChar * objname) {
  Vr_Exclude * cell = VG_(malloc)("vr.addExclude.1", sizeof(Vr_Exclude));
  cell->fnname  = VG_(strdup)("vr.addExclude.2", fnname);
  cell->objname = VG_(strdup)("vr.addExclude.3", objname);
  cell->used    = False;
  cell->next    = list;
  return cell;
}

Vr_Exclude * vr_findExclude (Vr_Exclude* list, HChar * fnname, HChar * objname) {
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
    Vr_Exclude *next = list->next;
    VG_(free)(list->fnname);
    VG_(free)(list->objname);
    VG_(free)(list);
    list = next;
  }
}

void vr_dumpExcludeList (Vr_Exclude * list, HChar * fname) {
  Int fd = VG_(fd_open)(fname,
                        VKI_O_CREAT|VKI_O_TRUNC|VKI_O_WRONLY,
                        VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IWGRP);
  VG_(umsg)("Dumping exclusions list to `%s'... ", fname);
  if (fd == -1) {
    VG_(umsg)("ERROR!\n");
    return;
  }

  Vr_Exclude * exclude;
  for (exclude = list ; exclude != NULL ; exclude = exclude->next) {
    VG_(write)(fd, exclude->fnname, VG_(strlen)(exclude->fnname));
    VG_(write)(fd, "\t", 1);
    VG_(write)(fd, exclude->objname, VG_(strlen)(exclude->objname));
    VG_(write)(fd, "\n", 1);
  }
  VG_(close)(fd);

  VG_(umsg)("OK.\n");
}

Vr_Exclude * vr_loadExcludeList (Vr_Exclude * list, HChar * fname) {
  VG_(umsg)("Loading exclusions list from `%s'... ", fname);
  Int fd = VG_(fd_open)(fname,VKI_O_RDONLY, 0);
  if (fd == -1) {
    VG_(umsg)("ERROR (open)\n");
    return list;
  }

  SizeT nLine = 256;
  HChar *line = VG_(malloc)("vr.loadExcludes.1", nLine*sizeof(HChar));
  Int lineno = 0;

  while (! VG_(get_line)(fd, &line, &nLine, &lineno)) {
    HChar * c;

    // Skip non-blank characters
    for (c = line;
         c<line+256 && *c != 0 && *c != '\t' && *c != ' ';
         ++c) {}
    if (*c == 0 || c>line+255) {
      VG_(umsg)("ERROR (parse)\n");
      return list;
    }
    *c = 0;

    // Skip blank characters
    for (++c;
         c<line+256 && *c != 0 && (*c == '\t' || *c == ' ');
         ++c) {}

    list = vr_addExclude (list,
                          /*fnname=*/ line,
                          /*objname*/ c);;
  }

  VG_(free)(line);
  VG_(close)(fd);

  VG_(umsg)("OK.\n");

  return list;
}

Bool vr_aboveFunction (HChar *ancestor, Addr * ips, UInt nips) {
  HChar fnname[10];
  UInt i;
  for (i = 1 ; i<nips ; ++i) {
    VG_(get_fnname)(ips[i], fnname, 10);
    if (VG_(strcmp)(fnname, ancestor) == 0) {
      return True;
    }
  }

  return False;
}


Bool vr_excludeIRSB(Vr_Exclude** list, Bool generate) {
  Addr ips[256];
  UInt nips = VG_(get_StackTrace)(VG_(get_running_tid)(),
                                  ips, 256,
                                  NULL, NULL,
                                  0);
  Addr addr = ips[0];

  HChar fnname[256];
  fnname[0] = 0;
  VG_(get_fnname)(addr, fnname, 255);

  // Never exclude unnamed functions
  if (fnname[0] == 0)
    return False;


  HChar objname[256];
  objname[0] = 0;
  VG_(get_objname)(addr, objname, 255);

  // Never exclude unnamed objects (maybe paranoia... does it even exist?)
  if (objname[0] == 0) {
    return False;
  }


  // Never exclude functions / objects unless they are explicitly listed
  Vr_Exclude *exclude = vr_findExclude (*list, fnname, objname);
  if (exclude == NULL) {
    if (generate && vr_aboveFunction("main", ips, nips)) {
      *list = vr_addExclude (*list, fnname, objname);
    }
    return False;
  }


  // Never exclude anything when generating the list
  if (generate)
    return False;


  // Inform the first time a rule is used
  if (!exclude->used) {
    VG_(umsg)("Using exclusion rule: %s\t%s\n", exclude->fnname, exclude->objname);
    exclude->used = True;
  }

  return True;
}
