
static VG_REGPARM(2) Long FCTNAME(64F,) (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double res;
  PREBACKEND;
  BACKENDFUNC(double)(*arg1, &res, CONTEXT);
  POSTBACKEND;
  Long *c = (Long*)(&res);
  return *c;
}

static VG_REGPARM(2) Long FCTCONVNAME(64F,) (Long a, Long b) {
  double *arg1 = (double*)(&a);
  float arg1f=*arg1;
  double res;
  float resf;
  PREBACKEND;
  BACKENDFUNC(float)(arg1f, &resf, CONTEXT);
  POSTBACKEND;
  res=resf;
  Long *c = (Long*)(&res);
  return *c;
}


static VG_REGPARM(0) void FCTNAME(64FLLO,) (void) {
  double res;
  PREBACKEND;
  BACKENDFUNC(double)(arg1CopyDouble[0], &res, CONTEXT);
  POSTBACKEND;
  arg1CopyDouble[0]=res;
}

static VG_REGPARM(0) void FCTCONVNAME(64FLLO,) (void) {
  float resf;
  float arg1f=arg1CopyDouble[0];
  PREBACKEND;
  BACKENDFUNC(float)(arg1f, &resf, CONTEXT);
  POSTBACKEND;
  arg1CopyDouble[0]=resf;
}


static VG_REGPARM(1) void FCTNAME(64Fx2,)(/*OUT*/V128* output) {
  const double* arg1=arg1CopyDouble;
  double* res=(double*) output;
  PREBACKEND;
  BACKENDFUNC(double)(arg1[0], res, CONTEXT);
  BACKENDFUNC(double)(arg1[1], res+1, CONTEXT);
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTCONVNAME(64Fx2,)(/*OUT*/V128* output) {
  float arg1f[2]={arg1CopyDouble[0],arg1CopyDouble[1]};
  double* res=(double*) output;
  float resf[2];
  PREBACKEND;
  BACKENDFUNC(float)(arg1f[0], resf, CONTEXT);
  BACKENDFUNC(float)(arg1f[1], resf+1, CONTEXT);
  POSTBACKEND;
  res[0]=resf[0];
  res[1]=resf[1];
}


static VG_REGPARM(1) void FCTNAME(64Fx4,) (/*OUT*/V256* output){
  double* res=(double*) output;
  PREBACKEND;
  for(int i=0; i<4; i++){
     BACKENDFUNC(double)(arg1CopyDouble[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTCONVNAME(64Fx4,) (/*OUT*/V256* output){
  float arg1f[4]={arg1CopyDouble[0],arg1CopyDouble[1],arg1CopyDouble[2],arg1CopyDouble[3]};
  double* res=(double*) output;
  float resf[4];

  PREBACKEND;
  for(int i=0; i<4; i++){
     BACKENDFUNC(float)(arg1f[i], resf+i, CONTEXT);
  }
  POSTBACKEND;
  res[0]=resf[0];
  res[1]=resf[1];
  res[2]=resf[2];
  res[3]=resf[3];
}


static VG_REGPARM(2) Int FCTNAME(32F,) (Long a) {
  float *arg1 = (float*)(&a);
  float res;
  PREBACKEND;
  BACKENDFUNC(float)(*arg1, &res, CONTEXT);
  POSTBACKEND;
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(0) void FCTNAME(32FLLO,) (void) {
  float res;
  PREBACKEND;
  BACKENDFUNC(float)(arg1CopyFloat[0], &res, CONTEXT);
  POSTBACKEND;
  arg1CopyFloat[0]=res;
}

static VG_REGPARM(1) void FCTNAME(32Fx8,) (/*OUT*/V256* output){
  float* res=(float*) output;
  float* arg1=arg1CopyFloat;
  PREBACKEND;
  for(int i=0; i<8; i++){
     BACKENDFUNC(float)(arg1[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTNAME(32Fx4,) (/*OUT*/V128* output){
  float* res=(float*) output;
  const float* arg1=arg1CopyFloat;
  PREBACKEND;
  for(int i=0; i<4;i++){
     BACKENDFUNC(float)(arg1[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}

