//FMA Operator
static VG_REGPARM(3) Long FCTNAME(64F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res;
  PREBACKEND;
  BACKEND(double, &res, *arg1, *arg2, *arg3);
  POSTBACKEND;
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Long FCTCONVNAME(64F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  float arg1f=*arg1;
  float arg2f=*arg2;
  float arg3f=*arg3;

  double res;
  float resf;
  PREBACKEND;
  BACKEND(float, &resf, arg1f, arg2f, arg3f);
  POSTBACKEND;
  res=resf;
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
  BACKEND(float, &res, *arg1, *arg2, *arg3);
  POSTBACKEND;
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}

//unFusED vERSION

static VG_REGPARM(3) Long FCTNAMEUNFUSED(64F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res;
  PREBACKEND;
  BACKEND_UNFUSED(double, &res, *arg1, *arg2, *arg3);
  POSTBACKEND;
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Long FCTCONVNAMEUNFUSED(64F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  float arg1f=*arg1;
  float arg2f=*arg2;
  float arg3f=*arg3;

  double res;
  float resf;
  PREBACKEND;
  BACKEND_UNFUSED(float, &resf, arg1f, arg2f, arg3f);
  POSTBACKEND;
  res=resf;
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int FCTNAMEUNFUSED(32F,) (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res;
  PREBACKEND;
  BACKEND_UNFUSED(float, &res, *arg1, *arg2, *arg3);
  POSTBACKEND;
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}
