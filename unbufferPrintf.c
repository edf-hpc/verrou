#include <stdio.h>

void __attribute__((constructor)) init_unbufferPrintf(void);
void __attribute__((constructor)) init_unbufferPrintf(void){
  setbuf(stdout,NULL);
};

void __attribute__((destructor)) finalyze_unbufferPrintf(void);
void __attribute__((destructor)) finalyze_unbufferPrintf(void){
};
