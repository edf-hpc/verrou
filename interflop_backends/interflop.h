#pragma once
/* interflop backend interface */
#include <stdarg.h>

/* interflop list of specific backend or frontend calls.
 * The instrumented code can request these operations through the function
 * below interflop_call */
enum INTERFLOP_CALL_OPCODE {
  CUSTOM = -1,
  SET_SEED = 0,
  SET_PRECISION,
  SET_MODE
};


/* interflop float compare predicates, follows the same order than
 * LLVM's FCMPInstruction predicates */
enum FCMP_PREDICATE {
  FCMP_FALSE,
  FCMP_OEQ,
  FCMP_OGT,
  FCMP_OGE,
  FCMP_OLT,
  FCMP_OLE,
  FCMP_ONE,
  FCMP_ORD,
  FCMP_UNO,
  FCMP_UEQ,
  FCMP_UGT,
  FCMP_UGE,
  FCMP_ULT,
  FCMP_ULE,
  FCMP_UNE,
  FCMP_TRUE,
};

/* Enumeration of types managed by function instrumentation */
enum FTYPES { FFLOAT, FDOUBLE, FFLOAT_PTR, FDOUBLE_PTR, FTYPES_END };

typedef struct interflop_function_info {
  // Indicate the identifier of the function
  char *id;
  // Indicate if the function is from library
  short isLibraryFunction;
  // Indicate if the function is intrinsic
  short isIntrinsicFunction;
  // Indicate if the function use double
  short useFloat;
  // Indicate if the function use float
  short useDouble;
} interflop_function_info_t;

/* Verificarlo call stack */
typedef struct interflop_function_stack {
  interflop_function_info_t **array;
  long int top;
} interflop_function_stack_t;

struct interflop_backend_interface_t {
  const char* (*backend_name)(void);
  const char* (*backend_version)(void);

  void (*add_float)(float a, float b, float *c, void *context);
  void (*sub_float)(float a, float b, float *c, void *context);
  void (*mul_float)(float a, float b, float *c, void *context);
  void (*div_float)(float a, float b, float *c, void *context);
  void (*sqrt_float)(float a, float *c, void *context);
  void (*cmp_float)(enum FCMP_PREDICATE p, float a, float b, int *c,
                              void *context);

  void (*add_double)(double a, double b, double *c, void *context);
  void (*sub_double)(double a, double b, double *c, void *context);
  void (*mul_double)(double a, double b, double *c, void *context);
  void (*div_double)(double a, double b, double *c, void *context);
  void (*sqrt_double)(double a, double *c, void *context);
  void (*cmp_double)(enum FCMP_PREDICATE p, double a, double b,
                               int *c, void *context);

  void (*cast_double_to_float)(double a, float *b, void *context);
  void (*madd_double)(double a, double b, double c, double *res, void *context);
  void (*madd_float)(float a, float b, float c, float *res, void *context);

  void (*enter_function)(interflop_function_stack_t *stack,
                                   void *context, int nb_args, va_list ap);

  void (*exit_function)(interflop_function_stack_t *stack,
                                  void *context, int nb_args, va_list ap);

  /* interflop_handle_call: called when the backend is one of the destinataries
   * of an interflop_call from user code */
  void *(*handle_call)( int destination,
		        enum INTERFLOP_CALL_OPCODE opcode,
                        void *context, ...);
  /* interflop_finalize: called at the end of the instrumented program
   * execution */
  void (*finalize)(void *context);
};

#define interflop_backend_empty_interface {	\
    NULL, NULL,					\
      NULL, NULL, NULL, NULL, NULL,		\
      NULL, NULL, NULL, NULL, NULL,		\
      NULL, NULL, NULL,				\
      NULL,					\
      NULL,					\
      NULL,					\
      NULL,					\
      NULL,                                     \
}

/* interflop_init: called at initialization before using a backend.
 * It returns an interflop_backend_interface_t structure with callbacks
 * for each of the numerical instrument hooks.
 *
 * argc: number of arguments passed to the backend
 *
 * argv: arguments passed to the backend, argv[0] always contains the name of
 * the backend library. argv[] may be deallocated after the call to
 * interflop_init. To make it persistent, a backend must copy it.
 *
 * context: the backend is free to make this point to a backend-specific
 * context. The frontend will pass the context back as the last argument of the
 * above instrumentation hooks.
 * */

struct interflop_backend_interface_t interflop_init(int argc, char **argv,
                                                    void **context);
