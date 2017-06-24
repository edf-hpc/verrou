/* interflop backend interface */


typedef float  floatx2[2];
typedef float  floatx4[4] ;
typedef float  floatx8[8];
typedef double doublex2[2];
typedef double doublex4[4] ;



struct interflop_backend_interface_t {
  void (*interflop_add_float)(float, float, float*, void*);
  void (*interflop_sub_float)(float, float, float*, void*);
  void (*interflop_mul_float)(float, float, float*, void*);
  void (*interflop_div_float)(float, float, float*, void*);

  void (*interflop_add_double)(double, double, double*, void*);
  void (*interflop_sub_double)(double, double, double*, void*);
  void (*interflop_mul_double)(double, double, double*, void*);
  void (*interflop_div_double)(double, double, double*, void*);

  void (*interflop_add_floatx2)(floatx2, floatx2, floatx2*, void*);
  void (*interflop_sub_floatx2)(floatx2, floatx2, floatx2*, void*);
  void (*interflop_mul_floatx2)(floatx2, floatx2, floatx2*, void*);
  void (*interflop_div_floatx2)(floatx2, floatx2, floatx2*, void*);

  void (*interflop_add_floatx4)(floatx4, floatx4, floatx4*, void*);
  void (*interflop_sub_floatx4)(floatx4, floatx4, floatx4*, void*);
  void (*interflop_mul_floatx4)(floatx4, floatx4, floatx4*, void*);
  void (*interflop_div_floatx4)(floatx4, floatx4, floatx4*, void*);

  void (*interflop_add_floatx8)(floatx8, floatx8, floatx8*, void*);
  void (*interflop_sub_floatx8)(floatx8, floatx8, floatx8*, void*);
  void (*interflop_mul_floatx8)(floatx8, floatx8, floatx8*, void*);
  void (*interflop_div_floatx8)(floatx8, floatx8, floatx8*, void*);

  void (*interflop_add_doublex2)(doublex2, doublex2, doublex2*, void*);
  void (*interflop_sub_doublex2)(doublex2, doublex2, doublex2*, void*);
  void (*interflop_mul_doublex2)(doublex2, doublex2, doublex2*, void*);
  void (*interflop_div_doublex2)(doublex2, doublex2, doublex2*, void*);

  void (*interflop_add_doublex4)(doublex4, doublex4, doublex4*, void*);
  void (*interflop_sub_doublex4)(doublex4, doublex4, doublex4*, void*);
  void (*interflop_mul_doublex4)(doublex4, doublex4, doublex4*, void*);
  void (*interflop_div_doublex4)(doublex4, doublex4, doublex4*, void*);


  void (*interflop_madd_float)(float, float, float, float*, void*);
  void (*interflop_madd_double)(double, double, double, double*, void*);

  

};

/* interflop_init: called at initialization before using a backend.
 * It returns an interflop_backend_interface_t structure with callbacks
 * for each of the numerical instrument hooks */

struct interflop_backend_interface_t interflop_BACKENDNAME_init(void ** context);
