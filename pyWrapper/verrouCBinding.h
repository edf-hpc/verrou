#pragma once

//C functions to call verrou client request

// start/stop instrumentation
void c_verrou_start_instrumentation(void);
void c_verrou_stop_instrumentation(void);

void c_verrou_start_soft_instrumentation(void);
void c_verrou_stop_soft_instrumentation(void);

// define derteministic section
void c_verrou_start_determinitic(int level);
void c_verrou_stop_determinitic(int level);

//dump cover
unsigned int c_verrou_dump_cover(void);


//counters
void c_verrou_display_counters(void);
unsigned int c_verrou_count_fp_instrumented(void);
unsigned int c_verrou_count_fp_not_instrumented(void);

void c_verrou_print_denorm_counter(void);
void c_verrou_reset_denorm_counter(void);
