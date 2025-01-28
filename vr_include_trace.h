#pragma once


typedef struct Vr_Include_Trace_ Vr_Include_Trace;
struct Vr_Include_Trace_ {
  HChar*      fnname;
  HChar*      objname;
  Vr_Include_Trace* next;
};


void vr_freeIncludeTraceList (Vr_Include_Trace* list) ;
Vr_Include_Trace * vr_loadIncludeTraceList (Vr_Include_Trace * list, const HChar * fname);
Bool vr_includeTraceIRSB (const HChar** fnname, const HChar **objname);
