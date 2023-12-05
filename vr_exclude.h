#pragma once

typedef struct Vr_Exclude_ Vr_Exclude;
struct Vr_Exclude_ {
  HChar*      fnname;
  HChar*      objname;
  Bool        used;
  Bool        counted;
  Vr_Exclude* next;
};

typedef struct Vr_IncludeSource_ Vr_IncludeSource;
struct Vr_IncludeSource_ {
  HChar*            fnname;
  HChar*            filename;
  UInt              linenum;
  Vr_IncludeSource* next;
};


void        vr_freeExcludeList (Vr_Exclude* list);
void        vr_dumpExcludeList (Vr_Exclude* list, const HChar* filename);
Vr_Exclude* vr_loadExcludeList (Vr_Exclude * list, const HChar * filename);
Vr_Exclude* vr_addObjectIfMatchPattern(Vr_Exclude * list, const HChar* objName);
Bool        vr_excludeIRSB(const HChar** fnname, const HChar** objname, Bool* counted);


void        vr_excludeIRSB_generate(const HChar** fnname, const HChar** objname);

void vr_freeIncludeSourceList (Vr_IncludeSource* list);
void vr_dumpIncludeSourceList (Vr_IncludeSource* list, const HChar* fname);
Vr_IncludeSource * vr_loadIncludeSourceList (Vr_IncludeSource * list, const HChar * fname);
Bool vr_includeSource (Vr_IncludeSource** list,
                       const HChar* fnname, const HChar* filename, UInt linenum);
void vr_includeSource_generate (Vr_IncludeSource** list,
				const HChar* fnname, const HChar* filename, UInt linenum);

Vr_IncludeSource * vr_addIncludeSource (Vr_IncludeSource* list, const HChar* fnname,
					const HChar * filename, UInt linenum);
Bool vr_includeSourceMutuallyExclusive( Vr_IncludeSource* listInclude, Vr_IncludeSource* listExclude);

void vr_generate_exclude_source(const char* functionName, int line, const char* fileName, const char* object );
Bool vr_clrIsInstrumented(const char* functionName, int line, const char* fileName, const char* object);
