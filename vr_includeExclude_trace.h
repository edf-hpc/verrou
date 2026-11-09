#pragma once


typedef struct Vr_IncludeExclude_Trace_ Vr_IncludeExclude_Trace;
struct Vr_IncludeExclude_Trace_ {
  HChar*      fnname;
  HChar*      objname;
  Vr_IncludeExclude_Trace* next;
};


void vr_freeIncludeExcludeTraceList (Vr_IncludeExclude_Trace* list) ;
Vr_IncludeExclude_Trace * vr_loadIncludeExcludeTraceList (Vr_IncludeExclude_Trace * list, const HChar * fname);
Bool vr_includeTraceIRSB (const HChar** fnname, const HChar **objname);
