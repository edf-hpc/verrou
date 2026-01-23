#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <float.h>
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

int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv){

   
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
   vprec_conf_t vprec_conf;
   //default init
   vprec_conf.precision_binary32=VPREC_PRECISION_BINARY32_DEFAULT;
   vprec_conf.range_binary32=VPREC_RANGE_BINARY32_DEFAULT;
   vprec_conf.precision_binary64=VPREC_PRECISION_BINARY64_DEFAULT;
   vprec_conf.range_binary64=VPREC_RANGE_BINARY64_DEFAULT;
   vprec_conf.mode=VPREC_MODE_DEFAULT;
   vprec_conf.preset=-1;
   vprec_conf.err_mode=vprec_err_mode_rel;
   vprec_conf.max_abs_err_exponent=  -DOUBLE_EXP_MIN;
   vprec_conf.daz=IFalse;
   vprec_conf.ftz=IFalse;

   //test case : double are float
   vprec_conf.precision_binary64=VPREC_PRECISION_BINARY32_DEFAULT;
   vprec_conf.range_binary64=VPREC_RANGE_BINARY32_DEFAULT;
   //test case : float are fp16
   vprec_conf.precision_binary32=10;
   vprec_conf.range_binary32=5;

   interflop_vprec_pre_init(my_panic , stderr , &context);
   interflop_vprec_configure((void*)&vprec_conf,context);
   interflop_vprec_init(context);


//   print_information_header(context);
   {
      float incTab[4]={7. , 6., 5. , 4. };
      double a=FLT_MAX;
      float af=(float)FLT_MAX;
      for( int i=0 ; i< 128 ; i++){
         printf("%d\n" , i);
         double doubleRes[4];
         float floatRes[4];
         for(int j=0; j<4; j++){
            interflop_vprec_add_double( a,(double)incTab[j], &(doubleRes[j]), context);
            floatRes[j]=af+incTab[j];

            if( ((double)floatRes[j]) != doubleRes[j]){
               printf("%.17f + %.17f => float: %.17f\tdouble: %.17f\n" , a, incTab[j],  floatRes[j],  doubleRes[j]);

               printf("%+.13a + %+.13a => float: %+.13a\tdouble: %+.13a\n" , a, incTab[j],  floatRes[j],  doubleRes[j]);
            }
         }
         for(int j=0; j<4; j++){
            incTab[j]*=2.f;
         }
      }
   }

#ifdef FLT16_MAX
   {
      _Float16 incTab[4]={7. , 6., 5. , 4. };
      float a=FLT16_MAX;
      _Float16 af=(_Float16)FLT16_MAX;
      for( int i=0 ; i< 50 ; i++){
         printf("%d\n" , i);
         float floatRes[4];
         _Float16 _Float16Res[4];
         for(int j=0; j<4; j++){
            interflop_vprec_add_float( a,(float)incTab[j],
&(floatRes[j]), context);
            _Float16Res[j]=af+incTab[j];

            if( ((float)_Float16Res[j]) != floatRes[j]){
               printf("%.17f + %.17f => _Float16: %.17f\tfloat: %.17f\n" , a, incTab[j],  _Float16Res[j],  floatRes[j]);

               printf("%+.13a + %+.13a => _Float16: %+.13a\tfloat: %+.13a\n" , a, incTab[j],  _Float16Res[j],  floatRes[j]);
            }
         }

         for(int j=0; j<4; j++){
            incTab[j]*=2.f;
         }
      }
   }
#endif
}
