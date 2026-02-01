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


mpfr_exp_t get_emax_for_mpfr(int range) {
  int emax = 1 << (range - 1);
  return emax;
}

mpfr_exp_t get_emin_for_mpfr(int range, int precision) {
  int emin = 1 << (range - 1);
  emin = -emin - precision + 3; // https://stackoverflow.com/questions/38664778/subnormal-numbers-in-different-precisions-with-mpfr
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

void get_smallest_normal_number(mpfr_t smallest_normal, int range) {
  mpfr_exp_t _emin = mpfr_get_emin();
  mpfr_exp_t _emax = mpfr_get_emax();

  int emax = (1 << (range - 1)) - 1;
  int emin = 1 - emax;

  mpfr_set_emin(mpfr_get_emin_min());
  mpfr_set_emax(mpfr_get_emax_max());

  mpfr_init2(smallest_normal, 256);
  /* 2 ^ (emin - precision)*/
  mpfr_set_ui_2exp(smallest_normal, 1, emin, MPFR_RNDN);

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

//#define DEBUG_CMP_FLOAT
#ifdef DEBUG_CMP_FLOAT
#include <assert.h>
#endif

void vprec_rounding(mpfr_t x, int range, int precision) {
#ifdef DEBUG_CMP_FLOAT
   assert(range==8);
   assert(precision==23);
   double d = mpfr_get_d(x, MPFR_RNDN);
   float fRef = mpfr_get_flt(x, MPFR_RNDN);
   float df=(float)d;
   assert(fRef==df);
#endif
   mpfr_clear_flags();
  mpfr_exp_t emax = get_emax_for_mpfr(range);
  mpfr_exp_t emin = get_emin_for_mpfr(range, precision);

  if( verbose_mode){
     printf("emax %ld\t emin %ld\n", emax, emin);
  }

  mpfr_t xRound;
  mpfr_set_emax(emax);
  mpfr_set_emin(emin);
  mpfr_inits2(precision+1,xRound, (mpfr_ptr)0);
  int i=mpfr_set(xRound, x,MPFR_RNDN);
  i = mpfr_check_range(xRound, i, MPFR_RNDN);
  i = mpfr_subnormalize(xRound, i, MPFR_RNDN);

  mpfr_t smallest_subnormal, smallest_normal,largest_normal;
  get_smallest_positive_subnormal_number(smallest_subnormal, range, precision);
  get_largest_positive_normal_number(largest_normal, range, precision);
  get_smallest_normal_number(smallest_normal,range);

  if (mpfr_cmpabs(xRound, largest_normal) > 0) {
    if (verbose_mode)
      fprintf(stderr, "Overflow detected\n");
    mpfr_set_inf(xRound, mpfr_sgn(x));
  }else{
     if (mpfr_cmpabs(xRound, smallest_normal)<0){
        if (verbose_mode){
           fprintf(stderr, "Subnormal detected limit(%e)\n",  mpfr_get_d(smallest_normal,MPFR_RNDN));
        }

        if (mpfr_cmpabs(xRound, smallest_subnormal) < 0) {
           if (verbose_mode)
              fprintf(stderr, "Underflow detected %e\n", mpfr_get_d(smallest_subnormal, MPFR_RNDN));
           mpfr_set_zero(xRound, mpfr_sgn(x));
        }
     }
  }

  mpfr_set(x, xRound,MPFR_RNDN);

#ifdef DEBUG_CMP_FLOAT
  double dRes = mpfr_get_d(xRound, MPFR_RNDN);
  if(dRes!=df && (! (dRes!=dRes && df!=df ) ) ){
     printf("d:%.17e  dRes: %.9e  float: %.9e\n", d, dRes, df);
     printf("d:%a  dRes: %a  float: %a\n", d, dRes, df);

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

     float fRes=(float)dRes;
     float ff=(float)df;

     if(ff==fRes){
        printf("Problem: sortie MPFR non representable en float\n");
     }
     int32_t resToPrint=*(int32_t*)(&fRes);
     int32_t fToPrint=  *(int32_t*)(&ff);
     printf("dRes: "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\n",
            BYTE_TO_BINARY(resToPrint>>24) ,
            BYTE_TO_BINARY(resToPrint >>16),
            BYTE_TO_BINARY(resToPrint>>8),
            BYTE_TO_BINARY(resToPrint ));
     printf("ff  : "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\n",
            BYTE_TO_BINARY(fToPrint>>24) ,
            BYTE_TO_BINARY(fToPrint >>16),
            BYTE_TO_BINARY(fToPrint>>8),
            BYTE_TO_BINARY(fToPrint ));
  };
#endif
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

#ifdef DEBUG_CMP_FLOAT
  int fpRangeMin[]={-1, 8};
  int fpRangeMax[]={-1, 8};

  int fpPrecisionMin[]={-1, 23};
  int fpPrecisionMax[]={-1, 23};
#else
  int fpRangeMin[]={2, 2};
  int fpRangeMax[]={8, 11};

  int fpPrecisionMin[]={2, 2};
  int fpPrecisionMax[]={23, 52};
#endif
  char opTabStr[]={'+','-','x','/', 's', 'f'}; //s for sqrt and f for fma

  vprec_mode_t modeTab[]={pb, ob, full};
  char* modeTabStr[]={"ib","ob","full"};

  for(int indexMode=0; indexMode<3 ; indexMode++){
     vprec_mode_t vprec_mode=modeTab[indexMode];
     printf("Generate %s reference\n", modeTabStr[indexMode]);

#ifdef DEBUG_CMP_FLOAT
     for(int indexType=1; indexType <2; indexType++){
#else
     for(int indexType=0; indexType <2; indexType++){
#endif
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
