/* interflop backend interface */
#pragma once



struct interflop_backend_interface_t {
  void (*interflop_add_float)(float, float, float*, void*);
  void (*interflop_sub_float)(float, float, float*, void*);
  void (*interflop_mul_float)(float, float, float*, void*);
  void (*interflop_div_float)(float, float, float*, void*);

  void (*interflop_add_double)(double, double, double*, void*);
  void (*interflop_sub_double)(double, double, double*, void*);
  void (*interflop_mul_double)(double, double, double*, void*);
  void (*interflop_div_double)(double, double, double*, void*);

  void (*interflop_cast_double_to_float)(double, float*, void*);

  void (*interflop_madd_float)(float, float, float, float*, void*);
  void (*interflop_madd_double)(double, double, double, double*, void*);
};

/* interflop_init: called at initialization before using a backend.
 * It returns an interflop_backend_interface_t structure with callbacks
 * for each of the numerical instrument hooks */

//struct interflop_backend_interface_t interflop_BACKENDNAME_init(void ** context);
