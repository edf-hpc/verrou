#pragma once

#define BACKTRACE_SIZE 40

typedef struct Vr_Exclude_Back_ Vr_Exclude_Back;
struct Vr_Exclude_Back_ {
   Int nbBack;
   Addr ip[BACKTRACE_SIZE];
   Bool used;
   Vr_Exclude_Back* next;
};

typedef struct Vr_Addr_List_ Vr_Addr_List;
struct Vr_Addr_List_ {
   Addr ip;
   Vr_Addr_List* next;
};


typedef struct Vr_Back_ Vr_Back;
struct Vr_Back_ {
   Vr_Exclude_Back* exclude;
   Vr_Exclude_Back* gen_exclude;
   Vr_Addr_List*    addr_list;
   HChar* gen_exclude_file;
   VgFile * handlerGenExclude;
   HChar* gen_addr_file;
   VgFile * handlerAddrList;
};


void vr_back_init(Vr_Back* vrBack, Bool genExclude, const HChar* rep);
void vr_back_finalize(Vr_Back* vrBack);


void vr_dumpBack(Vr_Back* vrBack);

void vr_loadBack(Vr_Back* vrBack, const HChar* filename);

void vr_addBackGen(Vr_Back* vrBack, Int nbBack, Addr* ip);


Bool vr_isInstrumentedBack(Vr_Back* vrBack, Int nbBack, Addr* ip);

