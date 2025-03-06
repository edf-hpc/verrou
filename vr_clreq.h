#pragma once

#include "vr_main.h"

Bool vr_handle_client_request (ThreadId tid, UWord *args, UWord *ret);
void vr_set_instrument_state (const HChar* reason, Vr_Instr state, Bool isHard);

void vr_print_denorm_counter(void);
void vr_reset_denorm_counter(void);
