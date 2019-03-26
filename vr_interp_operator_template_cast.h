

static VG_REGPARM(3) Int FCTNAME(64FTo32F,) (Long a) {
  double *arg1 = (double*)(&a);
  float res;
  BACKENDFUNC(double_to_float)(*arg1, &res,CONTEXT);
  Int *d = (Int*)(&res);
  return *d;
}
