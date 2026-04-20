#pragma once

#include <stdint.h>
#include "vr_exclude_back.h"


typedef struct Vr_Trace_Back_ Vr_Trace_Back;
struct Vr_Trace_Back_ {
   uint64_t hash;
   Addr bbAddr;
   Int nbBack;
   Addr ip[BACKTRACE_SIZE];
   uint64_t counter;
   uint64_t indexFirst;
   Vr_Trace_Back* next;
};


#define HASH_TABLE_TRACE_SIZE 256
#define HASH_TABLE_TRACE_MASK 0xff

typedef struct Vr_Trace_ Vr_Trace;
struct Vr_Trace_ {
   Vr_Trace_Back* trace[HASH_TABLE_TRACE_SIZE];
   Vr_Addr_List*    back_addr_list;
   Vr_Addr_List*    bb_addr_list;
   uint64_t currentIndexFirst;
   HChar* cover_file;
   VgFile * handlerCover;
   HChar* back_addr_file;
   VgFile * handlerBackAddrList;
   HChar* bb_addr_file;
   VgFile * handlerBBAddrList;

};


void vr_trace_init0(Vr_Trace* vrTrace);
void vr_trace_init_rep(Vr_Trace* vrTrace, const HChar* rep);
void vr_trace_finalize(Vr_Trace* vrTrace);


void vr_trace_dump_and_clear(Vr_Trace* vrTrace, UInt index);
void vr_trace_inc(Vr_Trace* vrTrace, Addr bbAddr, Int nbBack, Addr* ip);


Bool vr_trace_is_bbAddr_already_defined(Vr_Trace* vrTrace, Addr bbAddr);
void vr_trace_set_debugdata_for_bbAddr(Vr_Trace* vrTrace, Addr bbAddr,
                                       const char* filename, int linenum,
                                       Bool containFloat, Bool containCmp);

void vr_traceBackIRSB (IRSB* out,  Addr  addrBB);
