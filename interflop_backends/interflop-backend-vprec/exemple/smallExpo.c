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

#define MYPREC 4
#define MYRANGE 4   
   vprec_conf.precision_binary64=MYPREC;
   vprec_conf.range_binary64=MYRANGE;

   vprec_conf.precision_binary32=MYPREC;
   vprec_conf.range_binary32=MYRANGE;

   
   interflop_vprec_pre_init(my_panic , stderr , &context);
   interflop_vprec_configure((void*)&vprec_conf,context);
   interflop_vprec_init(context);

//   print_information_header(context);
   {
      #define NBELT 2

      //refLine:/ +0x1.a13088p-7 -0x1.91ab12p-10 => -0x1.a00000p+2
      char opTab[NBELT]={'f', '/'};
      double ref[NBELT]={ INFINITY, -0x1.a00000p+2};
      double aTab[NBELT]={+0x1.9f6e00p+4, +0x1.a13088p-7};
      double bTab[NBELT]={+0x1.13b986p+4, -0x1.91ab12p-10};
      double cTab[NBELT]={-0x1.b98b2ab369eccp-1, NAN};

      
      double resMPFRTab[NBELT];
      double resMPFRHighTab[NBELT];
      double resDoubleTab[NBELT];
      double resVprecDoubleTab[NBELT];
      float  resVprecFloatTab[NBELT];


      for(int i=0; i< NBELT ; i++){
         char op=opTab[i];
         double a=aTab[i];
         double b=bTab[i];
         double c=cTab[i];
         mpfr_set_default_prec(256);
         mpfr_t ma,mb,mc,mr, mr_ref;         
         mpfr_inits2(256, mr,mr_ref, ma, mb, mc,(mpfr_ptr)0);
         mpfr_set_default_prec(256);

         mpfr_set_d(ma, a, MPFR_RNDN);
         mpfr_set_d(mb, b, MPFR_RNDN);
         mpfr_set_d(mc, c, MPFR_RNDN);

         if( op=='+' ){
            resDoubleTab[i]=a+b;
            interflop_vprec_add_double( a, b, &(resVprecDoubleTab[i]), context);
            interflop_vprec_add_float( a, b, &(resVprecFloatTab[i]), context);
            mpfr_add(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='-' ){
            resDoubleTab[i]=a-b;
            interflop_vprec_sub_double( a, b, &(resVprecDoubleTab[i]), context);
            interflop_vprec_sub_float( a, b, &(resVprecFloatTab[i]), context);
            mpfr_sub(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='/' ){
            resDoubleTab[i]=a/b;
            interflop_vprec_div_double( a, b, &(resVprecDoubleTab[i]), context);
            interflop_vprec_div_float( a, b, &(resVprecFloatTab[i]), context);
            mpfr_div(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='*' ){
            resDoubleTab[i]=a*b;
            interflop_vprec_mul_double( a, b, &(resVprecDoubleTab[i]), context);
            interflop_vprec_mul_float( a, b, &(resVprecFloatTab[i]), context);
            mpfr_mul(mr, ma, mb, MPFR_RNDN);
         }

         if( op=='f' ){
            resDoubleTab[i]=fma(a,b,c);
            interflop_vprec_madd_double( a, b, c, &(resVprecDoubleTab[i]), context);
            interflop_vprec_madd_float( a, b, c, &(resVprecFloatTab[i]), context);
            mpfr_fma(mr, ma, mb, mc,MPFR_RNDN);
         }
         
         mpfr_set(mr_ref, mr,MPFR_RNDN);
         mpfr_prec_round(mr, MYPREC +1 , MPFR_RNDN);

         resMPFRTab[i]= mpfr_get_d(mr, MPFR_RNDN);
         resMPFRHighTab[i]= mpfr_get_d(mr_ref, MPFR_RNDN);

         if(resVprecFloatTab[i]!=resVprecDoubleTab[i]){
            printf("problem float/double(%d)  %+.13a  %+.13a \n",i, resVprecFloatTab[i],resVprecDoubleTab[i]);
         }

         if(resMPFRTab[i] !=ref[i]){
            printf("problem incompatible ref(%d)  %+.13a  %+.13a \n",i,resMPFRTab[i],ref[i]);            
         }
         if(resVprecFloatTab[i]!=ref[i]){
            printf("problem ref(%d)  %+.13a  %+.13a \n",i,resVprecFloatTab[i],ref[i]);

            printf("problem deltaRef(%d)  %+.13a  \n",i, resMPFRHighTab[i] - resMPFRTab[i]);
            
         }

      }
   }
}
