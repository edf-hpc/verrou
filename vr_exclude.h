#ifndef __VR_EXCLUDE_H
#define __VR_EXCLUDE_H

typedef struct Vr_Exclude_ Vr_Exclude;
struct Vr_Exclude_ {
  HChar*      fnname;
  HChar*      objname;
  Bool        used;
  Vr_Exclude* next;
};

Bool vr_aboveFunction (HChar *ancestor, Addr * ips, UInt nips);
Vr_Exclude* vr_findExclude (Vr_Exclude* list, HChar * fnname, HChar * objname);
Vr_Exclude* vr_addExclude (Vr_Exclude* list, HChar * fnname, HChar * objname);
void        vr_freeExcludeList (Vr_Exclude* list);
void        vr_dumpExcludeList (Vr_Exclude * list, HChar * filename);
Vr_Exclude* vr_loadExcludeList (Vr_Exclude * list, HChar * filename);
Bool        vr_excludeIRSB(Vr_Exclude ** list, Bool generate);

#endif /* ndef __VR_EXCLUDE_H */
