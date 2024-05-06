//FMA Operator
static VG_REGPARM(3) Long FCTNAME(64F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res;
  PREBACKEND;
  BACKENDFUNC(double)(*arg1, *arg2, SIGN *arg3, &res, CONTEXT);
  POSTBACKEND;
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int FCTNAME(32F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res;
  PREBACKEND;
  BACKENDFUNC(float)(*arg1, *arg2, SIGN *arg3, &res, CONTEXT);
  POSTBACKEND;
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}
