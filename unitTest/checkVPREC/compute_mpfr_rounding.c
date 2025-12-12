#include <errno.h>
#include <mpfr.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


typedef enum { ieee, pb, ob, full, unknown_mode } vprec_mode_t;
typedef enum { float_type, double_type, unknown_type } fptype_t;


static bool verbose_mode = false;
static int vprec_precision = -1;
static int vprec_range = -1;
static vprec_mode_t vprec_mode = unknown_mode;


mpfr_t largest_normal, smallest_subnormal;

#define HEADER_FMT "%-32s"
#define VAR_FMT "%22s"

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

void fprint_normalized_float(float x, FILE *stream, const char *nl) {
    fprintf(stream, "%+.6a%s", x, nl);
}
void fprint_normalized_double(double x, FILE *stream, const char *nl) {
   fprintf(stream, "%+.13a%s", x, nl);
}

void fprint_normalized_hex99(const char *msg, const char *name, mpfr_t x,
                             fptype_t type, FILE *stream) {
  fprintf(stream, HEADER_FMT " " VAR_FMT, msg, name);
  fprint_normalized_hex99_raw(x, type, stream, "\n");
}

void fprint_normalized_hex99_2(const char *msg, const char *name_x,
                               const char *name_y, mpfr_t x, mpfr_t y,
                               fptype_t type, FILE *stream) {
  fprintf(stream, HEADER_FMT " " VAR_FMT, msg, name_x);
  fprint_normalized_hex99_raw(x, type, stream, " ");
  fprintf(stream, VAR_FMT, name_y);
  fprint_normalized_hex99_raw(y, type, stream, "\n");
}

void print_normalized_hex99_debug(const char *msg, const char *name, mpfr_t x,
                                  fptype_t type) {
  fprint_normalized_hex99(msg, name, x, type, stderr);
}

void print_normalized_hex99(const char *msg, const char *name, mpfr_t x,
                            fptype_t type) {
  fprint_normalized_hex99(msg, name, x, type, stdout);
}

void print_normalized_hex99_debug_2(const char *msg, const char *name_x,
                                    const char *name_y, mpfr_t x, mpfr_t y,
                                    fptype_t type) {
  fprint_normalized_hex99_2(msg, name_x, name_y, x, y, type, stderr);
}

void print_debug(const char *msg, const char *name, const char *value) {
  fprintf(stderr, HEADER_FMT " " VAR_FMT "%-10s\n", msg, name, value);
}

void print_debug_float(const char *msg, const char *name, double x) {
  fprintf(stderr, HEADER_FMT " " VAR_FMT "%+.13a\n", msg, name, x);
}

void print_debug_int(const char *msg, const char *name, int x) {
  fprintf(stderr, HEADER_FMT " " VAR_FMT "%d\n", msg, name, x);
}


double get_float(const char *string) {
  errno = 0;
  char *endptr;
  double x = strtod(string, &endptr);
  if (errno != 0) {
    fprintf(stderr, "wrong float: %s", strerror(errno));
    exit(1);
  }
  return x;
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



long get_int(const char *string) {
  errno = 0;
  char *endptr;
  long x = strtol(string, &endptr, 10);
  if (errno != 0) {
    fprintf(stderr, "wrong int: %s", strerror(errno));
    exit(1);
  }
  return x;
}


mpfr_exp_t get_emax() {
  int range = vprec_range;
  int emax = 1 << (range - 1);
  return emax;
}

mpfr_exp_t get_emin() {
  int range = vprec_range;
  int precision = vprec_precision;
  int emin = 1 << (range - 1);
  emin = -emin - precision + 4;
  return emin;
}

void get_smallest_positive_subnormal_number(fptype_t type) {
  mpfr_exp_t _emin = mpfr_get_emin();
  mpfr_exp_t _emax = mpfr_get_emax();

  int range = vprec_range;
  int emax = (1 << (range - 1)) - 1;
  int emin = 1 - emax;
  // Precision withiout the implicit bit
  int precision = vprec_precision- 1;

  mpfr_set_emin(mpfr_get_emin_min());
  mpfr_set_emax(mpfr_get_emax_max());

  mpfr_init2(smallest_subnormal, 256);
  /* 2 ^ (emin - precision)*/
  mpfr_set_ui_2exp(smallest_subnormal, 1, emin - precision, MPFR_RNDN);

  mpfr_set_emin(_emin);
  mpfr_set_emax(_emax);

  if (verbose_mode) {
    print_normalized_hex99_debug(
        "[MPFR] (info)", "smallest subnormal = ", smallest_subnormal, type);
  }
}

void get_largest_positive_normal_number(fptype_t type) {

  mpfr_exp_t _emin = mpfr_get_emin();
  mpfr_exp_t _emax = mpfr_get_emax();

  int range = vprec_range;
  int emax = (1 << (range - 1)) - 1;
  // Precision withiout the implicit bit
  int precision = vprec_precision - 1;

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

  if (verbose_mode) {
    print_normalized_hex99_debug("[MPFR] (info)",
                                 "largest normal = ", largest_normal, type);
  }
}

void apply_operation(mpfr_t res, mpfr_t a, mpfr_t b, const char op,
                     fptype_t type) {
  int i = 0;
  switch (op) {
  case '+':
    i = mpfr_add(res, a, b, MPFR_RNDN);
    break;
  case '-':
    i = mpfr_sub(res, a, b, MPFR_RNDN);
    break;
  case 'x':
    i = mpfr_mul(res, a, b, MPFR_RNDN);
    break;
  case '/':
    i = mpfr_div(res, a, b, MPFR_RNDN);
    break;
  default:
    fprintf(stderr, "Bad op %c\n", op);
    exit(1);
  }
  if (verbose_mode) {
    print_normalized_hex99_debug("[MPFR] (operation)", "res = ", res, type);
  }
  mpfr_subnormalize(res, i, MPFR_RNDN);
  if (verbose_mode) {
    print_normalized_hex99_debug("[MPFR] (subnormalize)", "res = ", res, type);
  }
}

void intermediate_rounding(mpfr_t x, mpfr_t intermediate,
                           mpfr_prec_t precision) {
  mpfr_clear_flags();
  int i = mpfr_prec_round(x, precision, MPFR_RNDN);
  mpfr_set(intermediate, x, MPFR_RNDN);

  i = mpfr_check_range(x, i, MPFR_RNDN);

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

void compute2Args(double a, double b, char op, fptype_t type, FILE* fp) {
  const int working_precision = (type == float_type) ? 24 : 53;

  vprec_mode_t mode = vprec_mode;
  mpfr_prec_t precision = vprec_precision;
  mpfr_exp_t emax = get_emax();
  mpfr_exp_t emin = get_emin();

  mpfr_t ma, mb, mres, ma_inter, mb_inter, mres_inter;
  mpfr_inits2(working_precision, mres, ma, mb, ma_inter, mb_inter, mres_inter,
              (mpfr_ptr)0);

  if (type == float_type) {
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_set_flt(mb, b, MPFR_RNDN);
  } else if (type == double_type) {
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_set_d(mb, b, MPFR_RNDN);
  }

  if (verbose_mode) {
    print_normalized_hex99_debug_2("[MPFR] (before rounding)",
                                   "a = ", "b = ", ma, mb, type);
  }

  if (mode == pb || mode == full) {
    mpfr_clear_flags();
    mpfr_set_emax(emax);
    mpfr_set_emin(emin);
    intermediate_rounding(ma, ma_inter, precision);
    intermediate_rounding(mb, mb_inter, precision);

    if (verbose_mode) {
      print_normalized_hex99_debug_2("[MPFR] (intermediate rounding)",
                                     "a = ", "b = ", ma_inter, mb_inter, type);
      print_normalized_hex99_debug_2("[MPFR] (subnormalized)",
                                     "a = ", "b = ", ma, mb, type);
    }
  }

  if (verbose_mode) {
    print_normalized_hex99_debug_2("[MPFR] (after rounding)",
                                   "a = ", "b = ", ma, mb, type);
  }

  mpfr_clear_flags();
  mpfr_set_default_prec(256);
  mpfr_set_emax(mpfr_get_emax_max());
  mpfr_set_emin(mpfr_get_emin_min());


  apply_operation(mres, ma, mb, op, type);

  if (mode == ob || mode == full) {
    mpfr_clear_flags();
    mpfr_set_emax(emax);
    mpfr_set_emin(emin);
    intermediate_rounding(mres, mres_inter, precision);

    if (verbose_mode) {
      print_normalized_hex99_debug("[MPFR] (intermediate rounding)",
                                   "res = ", mres_inter, type);
      print_normalized_hex99_debug("[MPFR] (subnormalized)", "res = ", mres,
                                   type);
    }
  }

  fprintf(fp, "%c ", op);
  if(type==float_type){
     fprint_normalized_float(a, fp, " ");
     fprint_normalized_float(b, fp, " => ");
  }
  if(type==double_type){
     fprint_normalized_double(a, fp, " ");
     fprint_normalized_double(b, fp, " => ");
  }
  fprint_normalized_hex99_raw(mres, type,fp, "\n");
}


int main(int argc, char *argv[]) {

  if (argc != 3) {
    fprintf(stderr, "usage: compute_mpfr_rounding outputRep listOfPointFile\n");
    exit(1);
  }

  char* outputRep=argv[1];

  fptype_t fpTypeTab[]={float_type, double_type};
  char*  fpTypeTabStr[]={"float", "double"};
  int fpRangeMin[]={1, 1};
  int fpRangeMax[]={8, 11};

  int fpPrecisionMin[]={1, 1};
  int fpPrecisionMax[]={23, 52};

  char opTabStr[]={'+','-','x','/'};

  vprec_mode_t modeTab[]={pb, ob, full};
  char* modeTabStr[]={"ib","ob","full"};

  for(int indexMode=0; indexMode<3 ; indexMode++){
     vprec_mode=modeTab[indexMode];
     printf("Generate %s reference\n", modeTabStr[indexMode]);

     for(int indexType=0; indexType <2; indexType++){
        fptype_t fpType=fpTypeTab[indexType];
        printf("Generate %s reference\n", fpTypeTabStr[indexType]);

        for(vprec_range=fpRangeMin[indexType]; vprec_range<= fpRangeMax[indexType]; vprec_range++){
           for(vprec_precision=fpPrecisionMin[indexType]; vprec_precision<= fpPrecisionMax[indexType]; vprec_precision++){
              printf("\treference for E%dM%d\n", vprec_range, vprec_precision);

              get_largest_positive_normal_number(fpType);
              get_smallest_positive_subnormal_number(fpType);

              char line[512];
              FILE* listOfPoints=fopen(argv[2],"r");
              if(listOfPoints==NULL){
                 printf("Error while opening input point file\n");
                 return EXIT_FAILURE;
              }

              char outRefName[512];
              snprintf(outRefName, 512, "%s/mpfr_%s_%s_E%dM%d", outputRep, fpTypeTabStr[indexType], modeTabStr[indexMode], vprec_range, vprec_precision);
              printf("path:  %s\n", outRefName);
              FILE* refFile=fopen(outRefName,"w");

              while(fgets(line, 512, listOfPoints) ){
                 double args[2];
                 strLineToDoubleTab(args, 2, line);

                 for(int indexOp=0; indexOp< 4 ; indexOp++){
                    compute2Args(args[0], args[1], opTabStr[indexOp], fpType, refFile);
                 }
              }
              fclose(listOfPoints);
              fclose(refFile);
           }

        }
     }
  }
}
