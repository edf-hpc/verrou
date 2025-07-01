/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU Lesser General Public License is contained in the file COPYING.
*/


#include "verrouCBinding.h"
#define VERROU_SYNCHRO_INCLUDE
#include "verrou.h"

#include <stdio.h>

void c_verrou_start_instrumentation(void){
   VERROU_START_INSTRUMENTATION;
}

void c_verrou_stop_instrumentation(void){
   VERROU_STOP_INSTRUMENTATION;
}

void c_verrou_start_soft_instrumentation(void){
   VERROU_START_SOFT_INSTRUMENTATION;
}

void c_verrou_stop_soft_instrumentation(void){
   VERROU_STOP_SOFT_INSTRUMENTATION;
}


void c_verrou_start_determinitic(int level){
   VERROU_START_DETERMINISTIC(level);
}

void c_verrou_stop_determinitic(int level){
   VERROU_STOP_DETERMINISTIC(level);
}


void c_verrou_display_counters(void){
   VERROU_DISPLAY_COUNTERS;
}

unsigned int c_verrou_dump_cover(void){
   return VERROU_DUMP_COVER;
}


unsigned int c_verrou_count_fp_instrumented(void){
   unsigned int res=VERROU_COUNT_FP_INSTRUMENTED;
   return res;
}

unsigned int c_verrou_count_fp_not_instrumented(void){
   unsigned int res=VERROU_COUNT_FP_NOT_INSTRUMENTED;
   return res;
}

void c_verrou_print_denorm_counter(void){
   VERROU_PRINT_DENORM_COUNTER;
}

void c_verrou_reset_denorm_counter(void){
   VERROU_RESET_DENORM_COUNTER;
}


void c_verrou_set_rounding(char const*const rounding){
   VERROU_SET_ROUNDING_MODE(rounding);
}
