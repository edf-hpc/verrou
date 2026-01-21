
#include "../interflop_vprec.h"

//#include "../../interflop/interflop_stdlib.c"
//#include "../../interflop/iostream/logger.c"

//#include "../../interflop/sqrt/interflop_sqrt.c"
//#include "../../interflop/fma/interflop_fma.c"

#include <argp.h>
#include <assert.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <printf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void my_panic(const char *msg) {
  fprintf(stderr, "%s", msg);
  exit(1);
}
pid_t get_tid() { return syscall(__NR_gettid); }

long _vfc_strtol(const char *nptr, char **endptr, int *error) {
  *error = 0;
  errno = 0;
  long val = strtoll(nptr, endptr, 10);
  if (errno != 0) {
    *error = 1;
  }
  return val;
}

double _vfc_strtod(const char *nptr, char **endptr, int *error) {
  *error = 0;
  errno = 0;
  double val = strtod(nptr, endptr);
  if (errno != 0) {
    *error = 1;
  }
  return val;
}

int main(int argc, char**argv){

   
   interflop_set_handler("getenv", getenv);
   interflop_set_handler("sprintf", sprintf);
   interflop_set_handler("strerror", strerror);
   interflop_set_handler("gettid", get_tid);
   interflop_set_handler("fopen", fopen);
   interflop_set_handler("strcasecmp", strcasecmp);
   interflop_set_handler("vwarnx", vwarnx);
   interflop_set_handler("fprintf", fprintf);
   interflop_set_handler("exit", exit);
   interflop_set_handler("vfprintf", vfprintf);
   interflop_set_handler("malloc", malloc);
   interflop_set_handler("strcmp", strcmp);
   interflop_set_handler("strtol", _vfc_strtol);
   interflop_set_handler("strtod", _vfc_strtod);
   interflop_set_handler("strcpy", strcpy);
   interflop_set_handler("strncpy", strncpy);
   interflop_set_handler("fclose", fclose);
   interflop_set_handler("fgets", fgets);
   interflop_set_handler("strtok_r", strtok_r);
   interflop_set_handler("free", free);
   interflop_set_handler("calloc", calloc);
//   interflop_set_handler("gettimeofday", gettimeofday);
   
   void* context;
   double a=0.1;
   float af=0.1;
   double b=0.01;
   float bf=0.01;

   double cRef=a+b;
   float cfRef=af+bf;
   
   double c;
   float cf;

   interflop_vprec_pre_init(my_panic , stderr , &context);
   interflop_vprec_init(context);
   
//   print_information_header(context);
   
   interflop_vprec_add_double( a,b, &c,context);
   interflop_vprec_add_float( af,bf,&cf, context);

   printf("c: %.18f  %.18f\n", c, cRef);
   printf("cf: %.18f  %.18f\n", cf, cfRef);
   
}
