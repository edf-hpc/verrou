#include <errno.h>
#include <mpfr.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

typedef enum { ieee, pb, ob, full, unknown_mode } vprec_mode_t;
typedef enum { float_type, double_type, unknown_type } fptype_t;


static bool verbose_mode = false;



void fprint_normalized_hex99_raw(mpfr_t x, fptype_t type, FILE *stream,
                                 const char *nl) {
  if (type == float_type) {
    float f = mpfr_get_flt(x, MPFR_RNDN);
    fprintf(stream, "%+.6a%s", f, nl);
  } else if (type == double_type) {
    double d = mpfr_get_d(x, MPFR_RNDN);
    fprintf(stream, "%+.13a%s", d, nl);
  }
}

void strLineToDoubleTab(double* tab, int nbArgs,  char *string) {
  errno = 0;
  char *endptr=string;
  for(int i=0; i< nbArgs ; i++){
     tab[i]=strtod(endptr, &endptr);
     if (errno != 0) {
        fprintf(stderr, "wrong float: %s", strerror(errno));
        exit(1);
     }
  }
}


mpfr_exp_t get_emax(int range) {
  int emax = 1 << (range - 1);
  return emax;
}

mpfr_exp_t get_emin(int range, int precision) {
  int emin = 1 << (range - 1);
  emin = -emin - precision + 4; //why?
  return emin;
}

void get_smallest_positive_subnormal_number(mpfr_t smallest_subnormal, int range, int precision) {
  mpfr_exp_t _emin = mpfr_get_emin();
  mpfr_exp_t _emax = mpfr_get_emax();

  int emax = (1 << (range - 1)) - 1;
  int emin = 1 - emax;

  mpfr_set_emin(mpfr_get_emin_min());
  mpfr_set_emax(mpfr_get_emax_max());

  mpfr_init2(smallest_subnormal, 256);
  /* 2 ^ (emin - precision)*/
  mpfr_set_ui_2exp(smallest_subnormal, 1, emin - precision, MPFR_RNDN);

  mpfr_set_emin(_emin);
  mpfr_set_emax(_emax);
}

void get_largest_positive_normal_number(mpfr_t largest_normal, int range, int precision) {

  mpfr_exp_t _emin = mpfr_get_emin();
  mpfr_exp_t _emax = mpfr_get_emax();

  int emax = (1 << (range - 1)) - 1;

  mpfr_set_emin(mpfr_get_emin_min());
  mpfr_set_emax(mpfr_get_emax_max());

  mpfr_init2(largest_normal, 256);

  /* 2^(-precision) */
  mpfr_set_ui_2exp(largest_normal, 1, -precision, MPFR_RNDN);
  /* 2 - 2^(-precision) */
  mpfr_ui_sub(largest_normal, 2, largest_normal, MPFR_RNDN);
  /* (2 - 2^(-precision)) * 2^(emax) */
  mpfr_mul_2ui(largest_normal, largest_normal, emax, MPFR_RNDN);

  mpfr_set_emin(_emin);
  mpfr_set_emax(_emax);

}

int nb_args_from_op(const char op){
 switch (op) {
  case '+':
  case '-':
  case 'x':
  case '/':
    return 2;
  case 's':
     return 1;
  case 'f':
     return 3;
  default:
    fprintf(stderr, "Bad op %c\n", op);
    exit(1);
  }
}


void apply_operation(mpfr_t res, mpfr_t* args, const char op,
                     fptype_t type) {
   mpfr_clear_flags();
   mpfr_set_default_prec(256);
   mpfr_set_emax(mpfr_get_emax_max());
   mpfr_set_emin(mpfr_get_emin_min());

   int i = 0;
   switch (op) {
   case '+':
      i = mpfr_add(res, args[0], args[1], MPFR_RNDN);
      break;
   case '-':
      i = mpfr_sub(res, args[0], args[1], MPFR_RNDN);
      break;
   case 'x':
      i = mpfr_mul(res, args[0], args[1], MPFR_RNDN);
      break;
   case '/':
      i = mpfr_div(res, args[0], args[1], MPFR_RNDN);
      break;
   case 's':
   {
      double d = mpfr_get_d(args[0], MPFR_RNDN);
      if(d>=0 ){
         i = mpfr_sqrt(res, args[0], MPFR_RNDN);
      }else{
         d= - NAN;
         mpfr_set_d(res, d, MPFR_RNDN);
      }
      break;
   }
   case 'f':
      i = mpfr_fma(res, args[0], args[1], args[2], MPFR_RNDN);
      break;
   default:
      fprintf(stderr, "Bad op %c\n", op);
      exit(1);
   }
   i = mpfr_check_range(res, i, MPFR_RNDN);
   mpfr_subnormalize(res, i, MPFR_RNDN);
}

void vprec_rounding(mpfr_t x, int range, int precision) {
  mpfr_clear_flags();
  mpfr_exp_t emax = get_emax(range);
  mpfr_exp_t emin = get_emin(range,precision);

  mpfr_set_emax(emax);
  mpfr_set_emin(emin);
  int i = mpfr_prec_round(x, precision+1, MPFR_RNDN);

  i = mpfr_check_range(x, i, MPFR_RNDN);

  mpfr_t smallest_subnormal, largest_normal;
  get_smallest_positive_subnormal_number(smallest_subnormal, range, precision);
  get_largest_positive_normal_number(largest_normal, range, precision);

  if (mpfr_cmpabs(x, largest_normal) > 0) {
    if (verbose_mode)
      fprintf(stderr, "Overflow detected\n");
    mpfr_set_inf(x, mpfr_sgn(x));
  } else if (mpfr_cmpabs(x, smallest_subnormal) < 0) {
    if (verbose_mode)
      fprintf(stderr, "Underflow detected\n");
    mpfr_set_zero(x, mpfr_sgn(x));
  }

  i = mpfr_subnormalize(x, i, MPFR_RNDN);
}


void generate_ref(double* arg, char op, fptype_t type, FILE* fp,
                  vprec_mode_t mode, int range, int precision) {
  const int working_precision = (type == float_type) ? 24 : 53;

  int nbArg=nb_args_from_op(op);
  mpfr_t marg[3];
  mpfr_t  mres;
  mpfr_t marg_org[3];
  mpfr_inits2(working_precision, mres,(mpfr_ptr)0);
  for(int i=0; i< nbArg; i++){
     mpfr_inits2(working_precision, marg[i],(mpfr_ptr)0);
     if (type == float_type) {
        mpfr_set_flt(marg[i], arg[i], MPFR_RNDN);
     } else if (type == double_type) {
        mpfr_set_d(marg[i], arg[i], MPFR_RNDN);
     }
     mpfr_inits2(working_precision, marg_org[i],(mpfr_ptr)0);
     mpfr_set(marg_org[i], marg[i],MPFR_RNDN);
  }

  if (mode == pb || mode == full) {
    for(int i=0; i< nbArg; i++){
       vprec_rounding(marg[i], range, precision);
    }
  }

  apply_operation(mres, marg, op, type);

  if (mode == ob || mode == full) {
    vprec_rounding(mres,range,precision);
  }

  fprintf(fp, "%c ", op);
  for(int i=0; i< nbArg-1; i++){
     fprint_normalized_hex99_raw(marg_org[i], type,fp, " ");
  }
  fprint_normalized_hex99_raw(marg_org[nbArg-1], type,fp, " => ");
  fprint_normalized_hex99_raw(mres, type,fp, "\n");
}




int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "usage: compute_mpfr_rounding outputRep\n");
    exit(1);
  }

  char* outputRep=argv[1];

  fptype_t fpTypeTab[]={float_type, double_type};
  char*  fpTypeTabStr[]={"float", "double"};
  int fpRangeMin[]={2, 2};
  int fpRangeMax[]={8, 11};

  int fpPrecisionMin[]={2, 2};
  int fpPrecisionMax[]={23, 52};

  char opTabStr[]={'+','-','x','/', 's', 'f'}; //s for sqrt and f for fma

  vprec_mode_t modeTab[]={pb, ob, full};
  char* modeTabStr[]={"ib","ob","full"};

  for(int indexMode=0; indexMode<3 ; indexMode++){
     vprec_mode_t vprec_mode=modeTab[indexMode];
     printf("Generate %s reference\n", modeTabStr[indexMode]);

     for(int indexType=0; indexType <2; indexType++){
        fptype_t fpType=fpTypeTab[indexType];
        printf("Generate %s reference\n", fpTypeTabStr[indexType]);

        for(int vprec_range=fpRangeMin[indexType]; vprec_range<= fpRangeMax[indexType]; vprec_range++){
           for(int vprec_precision=fpPrecisionMin[indexType]; vprec_precision<= fpPrecisionMax[indexType]; vprec_precision++){
              printf("\treference for E%dM%d\n", vprec_range, vprec_precision);

              char line[512];
              FILE* listOfPoints=NULL;
              char inputPointFile[512];
              snprintf(inputPointFile, 512, "input_%i.txt",vprec_range);
              if(inputPointFile==NULL){
                 printf("Error while creating input filename name\n");
                 return EXIT_FAILURE;
              }

              listOfPoints=fopen(inputPointFile,"r");

              if(listOfPoints==NULL){
                 printf("Error while opening input point file\n");
                 return EXIT_FAILURE;
              }

              char outRefName[512];
              snprintf(outRefName, 512, "%s/mpfr_%s_%s_E%dM%d", outputRep, fpTypeTabStr[indexType], modeTabStr[indexMode], vprec_range, vprec_precision);
              printf("path:  %s\n", outRefName);
              FILE* refFile=fopen(outRefName,"w");

              while(fgets(line, 512, listOfPoints) ){
                 double args[3];
                 strLineToDoubleTab(args, 3, line);

                 for(int indexOp=0; indexOp< 6 ; indexOp++){
                    generate_ref(args, opTabStr[indexOp], fpType, refFile, vprec_mode, vprec_range, vprec_precision);
                 }
              }
              fclose(listOfPoints);
              fclose(refFile);
           }

        }
     }
  }
}
