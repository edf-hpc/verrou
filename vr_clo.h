#pragma once

#include "vr_main.h"

void vr_env_clo_one_option (const HChar* env, const HChar *clo);
void vr_env_clo (void);
void vr_clo_defaults (void);
Bool vr_process_clo (const HChar *arg);
void vr_print_usage (void);
void vr_print_debug_usage (void);
