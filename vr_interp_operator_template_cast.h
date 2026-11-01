

static VG_REGPARM(3) Int FCTNAME(64FTo32F,) (Long a) {
  double *arg1 = (double*)(&a);
  float res;
  PREBACKEND;
  BACKEND(double_to_float, &res, *arg1);
  POSTBACKEND;
  Int *d = (Int*)(&res);
  return *d;
}
