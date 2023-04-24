#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_NEAREST

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_UPWARD

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_DOWNWARD

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_FARTHEST

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_ZERO

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_RANDOM

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_RANDOM_DET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_RANDOM_COMDET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_RANDOM_SCOMDET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_AVERAGE

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_AVERAGE_DET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_AVERAGE_COMDET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_AVERAGE_SCOMDET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_PRANDOM

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_PRANDOM_DET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_PRANDOM_COMDET

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_SR_MONOTONIC

  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
#undef IFV_FCTNAME
#define IFV_FCTNAME(FCT) interflop_verrou_##FCT
