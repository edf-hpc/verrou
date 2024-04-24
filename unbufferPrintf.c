#include <stdio.h>

void __attribute__((constructor)) init_unbufferPrintf(){
  setbuf(stdout,NULL);
}

void __attribute__((destructor)) finalyze_unbufferPrintf(){
};
