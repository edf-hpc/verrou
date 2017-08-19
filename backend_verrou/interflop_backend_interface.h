/* interflop backend interface */



#define vector_interflop_definition(simdtype,basictype,size) typedef basictype simdtype __attribute__ ((vector_size(sizeof(basictype)*(size))))


vector_interflop_definition(floatx2,float,2);
vector_interflop_definition(floatx4,float,4);
vector_interflop_definition(floatx8,float,8);

vector_interflop_definition(doublex2,double,2);
vector_interflop_definition(doublex4,double,4);



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

  void (*interflop_cast_double_to_float)(double, float*, void*);

  void (*interflop_madd_float)(float, float, float, float*, void*);
  void (*interflop_madd_double)(double, double, double, double*, void*);

  

};

/* interflop_init: called at initialization before using a backend.
 * It returns an interflop_backend_interface_t structure with callbacks
 * for each of the numerical instrument hooks */

//struct interflop_backend_interface_t interflop_BACKENDNAME_init(void ** context);
