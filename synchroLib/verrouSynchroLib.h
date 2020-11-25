#pragma once

/*
List of env variable :
DEBUG_PRINT_SYNCHRO
SYNCHRO_LIST
GENERATE_SYNCHRO_LIST
GENERATE_SYNCHRO_FP_LIST
 */


#ifdef __cplusplus
extern "C" {
#endif
  void verrou_synchro_init();
  void verrou_synchro(char const*const key, int index);
  void verrou_synchro_finalyze();
#ifdef __cplusplus
};
#endif


#define VERROU_SYNCHRO_INIT verrou_synchro_init();
#define VERROU_SYNCHRO(a,b) verrou_synchro(a,b);
#define VERROU_SYNCHRO_FINALIZE verrou_synchro_finalyze();
