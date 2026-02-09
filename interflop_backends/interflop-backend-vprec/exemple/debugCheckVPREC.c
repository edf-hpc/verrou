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



#define PREC 4
#define RANGE 4

//float
#define TYPEINT 0
//#define TYPEINT 1 //double


#if TYPEINT == 0
#define MYTYPE float
#define VPREC_ADD interflop_vprec_add_float
#define VPREC_SUB interflop_vprec_sub_float
#define VPREC_MUL interflop_vprec_mul_float
#define VPREC_DIV interflop_vprec_div_float
#define VPREC_MADD interflop_vprec_madd_float
#else
#define MYTYPE double
#define VPREC_ADD interflop_vprec_add_double
#define VPREC_SUB interflop_vprec_sub_double
#define VPREC_MUL interflop_vprec_mul_double
#define VPREC_DIV interflop_vprec_div_double
#define VPREC_MADD interflop_vprec_madd_double
#endif



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
   vprec_conf.mode=vprecmode_full;
//   vprec_conf.mode=vprecmode_ib;
   vprec_conf.preset=-1;
   vprec_conf.err_mode=vprec_err_mode_rel;
   vprec_conf.max_abs_err_exponent=  -DOUBLE_EXP_MIN;
   vprec_conf.daz=IFalse;
   vprec_conf.ftz=IFalse;

   vprec_conf.precision_binary32=PREC;
   vprec_conf.range_binary32=RANGE;
   vprec_conf.precision_binary64=PREC;
   vprec_conf.range_binary64=RANGE;

   interflop_vprec_pre_init(my_panic , stderr , &context);
   interflop_vprec_configure((void*)&vprec_conf,context);
   interflop_vprec_init(context);

//   print_information_header(context);
   {

   /* ref : +0x1.f00000p-2	4.84375000e-01 */
   /*       vprec : +0x1.d00000p-2	4.53125000e-01 */
   /*       rel : +0x1.084210p-4	6.45161271e-02	2.00ulp */
   /*       abs error : +0x1.000000p-5	3.12500000e-02 */
   /*    Error : resVprec [+0x1.d00000p-2] != ref [+0x1.f00000p-2] */

   /*    refLine:= f -0x1.98c154p-1 -0x1.d9e998p-1 -0x1.236392p-2 => +0x1.f00000p-2 */
   /*   f -0x1.98c154p-1 -0x1.d9e998p-1 -0x1.236392p-2 => +0x1.f00000p-2 */

#define NBELT 2
      MYTYPE ref[NBELT]={ +0x1.f00000p-2, +0x1.f00000p-2};
      char opTab[NBELT]={'f', 'f'};
      MYTYPE aTab[NBELT]={-0x1.98c154p-1, -0x1.98c154p-1};
      MYTYPE bTab[NBELT]={-0x1.d9e998p-1, -0x1.d9e998p-1};
      MYTYPE cTab[NBELT]={-0x1.236392p-2, -0x1.236392p-2};

      MYTYPE resMPFRTab[NBELT];
      MYTYPE resTypeTab[NBELT];
      MYTYPE resVprecTab[NBELT];

      for(int i=0; i< NBELT ; i++){
         char op=opTab[i];
         MYTYPE a=aTab[i];
         MYTYPE b=bTab[i];
         MYTYPE c=cTab[i];
         mpfr_t ma,mb,mc,mr;
         mpfr_inits2(256, mr, ma, mb, mc, (mpfr_ptr)0);

         mpfr_set_default_prec(256);
#if MYTYPE==float
         mpfr_set_flt(ma, a, MPFR_RNDN);
         mpfr_set_flt(mb, b, MPFR_RNDN);
         mpfr_set_flt(mc, c, MPFR_RNDN);
#else
         mpfr_set_d(ma, a, MPFR_RNDN);
         mpfr_set_d(mb, b, MPFR_RNDN);
         mpfr_set_d(mc, c, MPFR_RNDN);
#endif
         if( op=='+' ){
            resTypeTab[i]=a+b;
            VPREC_ADD ( a, b, &(resVprecTab[i]), context);
            mpfr_add(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='-' ){
            resTypeTab[i]=a-b;
            VPREC_SUB ( a, b, &(resVprecTab[i]), context);
            mpfr_sub(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='/' ){
            resTypeTab[i]=a/b;
            VPREC_DIV( a, b, &(resVprecTab[i]), context);
            mpfr_div(mr, ma, mb, MPFR_RNDN);
         }
         if( op=='*' ){
            resTypeTab[i]=a*b;
            VPREC_MUL( a, b, &(resVprecTab[i]), context);
            mpfr_mul(mr, ma, mb, MPFR_RNDN);
         }

         if( op=='f' ){
            resTypeTab[i]=fma(a,b,c);
            VPREC_MADD( a, b,c, &(resVprecTab[i]), context);
            mpfr_fma(mr, ma, mb, mc, MPFR_RNDN);
         }

#if TYPE==float
         resMPFRTab[i]= mpfr_get_flt(mr, MPFR_RNDN);
#else
         resMPFRTab[i]= mpfr_get_d(mr, MPFR_RNDN);
#endif

         if(resVprecTab[i]!=ref[i]){
            printf("problem Vprec/ref %d\n", i);
            printf("\tref %+.13a  %.17e \n",ref[i],ref[i]);
            printf("\tVPREC %+.13a  %.17e \n",resVprecTab[i],resVprecTab[i]);
            printf("\tMPFR  %+.13a   %+.17e\n",resMPFRTab[i],resMPFRTab[i]);
            printf("\tnatif %+.13a   %+.17e\n",resTypeTab[i],resTypeTab[i]);           
         }
      }
   }
}
