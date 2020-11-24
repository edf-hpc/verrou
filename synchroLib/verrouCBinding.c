#include "verrouCBinding.h"
#define VERROU_SYNCHRO_INCLUDE
#include "verrou.h"

#include <stdio.h>

void c_verrou_start_instrumentation(){
   VERROU_START_INSTRUMENTATION;
}

void c_verrou_stop_instrumentation(){
   VERROU_STOP_INSTRUMENTATION;
}

void c_verrou_start_determinitic(int level){
   VERROU_START_DETERMINISTIC(level);
}

void c_verrou_stop_determinitic(int level){
   VERROU_STOP_DETERMINISTIC(level);
}


void c_verrou_display_counters(){
   VERROU_DISPLAY_COUNTERS;
}

unsigned int c_verrou_dump_cover(){
   return VERROU_DUMP_COVER;
}


unsigned int c_verrou_count_fp_instrumented(){
   unsigned int res=VERROU_COUNT_FP_INSTRUMENTED;
   return res;
}

unsigned int c_verrou_count_fp_not_instrumented(){
   unsigned int res=VERROU_COUNT_FP_NOT_INSTRUMENTED;
   return res;
}
