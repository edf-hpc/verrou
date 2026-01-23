#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <float.h>
#include "../interflop_vprec.h"
#include <mpfr.h>

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

   interflop_vprec_pre_init(my_panic , stderr , &context);
   interflop_vprec_configure((void*)&vprec_conf,context);
   interflop_vprec_init(context);

//   print_information_header(context);
   {
      #define NBELT 7

//refLine:/ +0x1.ef39685e5dd62p+125 -0x1.39523036f42e8p+123 => -0x1.949fe80000000p+2  
//refLine:+ +0x1.fa4666b7a3ec0p+124 -0x1.3f318cd711e1ap+125 => -0x1.0839640000000p+123
// refLine:/ +0x1.fa4666b7a3ec0p+124 -0x1.3f318cd711e1ap+125 => -0x1.960b140000000p-1 
//    refLine:+ -0x1.109a3406e73f8p-1 -0x1.811b54591977cp-2 => -0x1.d127e00000000p-1  
//    refLine:- -0x1.12918d5474aa8p-1 +0x1.8bd1127b4a6e0p-5 => -0x1.2b4ea00000000p-1  
//    refLine:x -0x1.12918d5474aa8p-1 +0x1.8bd1127b4a6e0p-5 => -0x1.a886d00000000p-6  
//    refLine:/ -0x1.12918d5474aa8p-1 +0x1.8bd1127b4a6e0p-5 => -0x1.6329840000000p+3  

//     refLine:f -0x1.12918d5474aa8p-1 +0x1.8bd1127b4a6e0p-5 -0x1.b98b2ab369eccp-1 => -0x1.c6cf600000000p-1

      double ref[NBELT]={-0x1.949fe80000000p+2,-0x1.0839640000000p+123, -0x1.960b140000000p-1,-0x1.d127e00000000p-1 ,-0x1.2b4ea00000000p-1 ,-0x1.a886d00000000p-6 ,-0x1.6329840000000p+3 };      
      char opTab[NBELT]={'/', '+', '/', '+', '-', '*','/' };
      double aTab[NBELT]={+0x1.ef39685e5dd62p+125,  +0x1.fa4666b7a3ec0p+124,  +0x1.fa4666b7a3ec0p+124,  -0x1.109a3406e73f8p-1,  -0x1.12918d5474aa8p-1 , -0x1.12918d5474aa8p-1,  -0x1.12918d5474aa8p-1};
      double bTab[NBELT]={-0x1.39523036f42e8p+123,  -0x1.3f318cd711e1ap+125, -0x1.3f318cd711e1ap+125,  -0x1.811b54591977cp-2, +0x1.8bd1127b4a6e0p-5 ,  +0x1.8bd1127b4a6e0p-5,  +0x1.8bd1127b4a6e0p-5};


      double resMPFRTab[NBELT];

      double resDoubleTab[NBELT];
      double resFloatTab[NBELT];
      double resVprecTab[NBELT];

      for(int i=0; i< NBELT ; i++){
         char op=opTab[i];
         double a=aTab[i];
         double b=bTab[i];
         mpfr_t ma,mb,mr;
         mpfr_inits2(256, mr, ma, mb, (mpfr_ptr)0);

         mpfr_set_default_prec(256);
         mpfr_set_d(ma, a, MPFR_RNDN);
         mpfr_set_d(mb, b, MPFR_RNDN);

         if( op=='+' ){
            resDoubleTab[i]=a+b;
            interflop_vprec_add_double( a, b, &(resVprecTab[i]), context);
            mpfr_add(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='-' ){
            resDoubleTab[i]=a-b;
            interflop_vprec_sub_double( a, b, &(resVprecTab[i]), context);
            mpfr_sub(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='/' ){
            resDoubleTab[i]=a/b;
            interflop_vprec_div_double( a, b, &(resVprecTab[i]), context);
            mpfr_div(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='*' ){
            resDoubleTab[i]=a*b;
            interflop_vprec_mul_double( a, b, &(resVprecTab[i]), context);
            mpfr_mul(mr, ma, mb, MPFR_RNDN);
         }
         resFloatTab[i]=(double)((float)resDoubleTab[i]);

         mpfr_prec_round(mr, VPREC_PRECISION_BINARY32_DEFAULT +1 , MPFR_RNDN);
         resMPFRTab[i]= mpfr_get_d(mr, MPFR_RNDN);

         if(resFloatTab[i]!=resVprecTab[i]){
            printf("problem %d\n",i);
         }

         if(resFloatTab[i]!=ref[i]){
            printf("problem ref %d  %+.13a  %+.13a \n",i,resFloatTab[i],ref[i]);
         }

         if(resFloatTab[i]!=resMPFRTab[i]){
            printf("problem MPFR %d  %+.13a  %+.13a \n",i,resFloatTab[i],resMPFRTab[i]);
         }
      }
   }
}
