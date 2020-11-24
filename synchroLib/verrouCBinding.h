#pragma once

//C functions to call verrou client request

// start/stop instrumentation
void c_verrou_start_instrumentation();
void c_verrou_stop_instrumentation();

// define derteministic section
void c_verrou_start_determinitic(int level);
void c_verrou_stop_determinitic(int level);

//dump cover
unsigned int c_verrou_dump_cover();


//counters
void c_verrou_display_counters();
unsigned int c_verrou_count_fp_instrumented();
unsigned int c_verrou_count_fp_not_instrumented();
