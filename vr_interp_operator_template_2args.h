
static VG_REGPARM(2) Long FCTNAME(64F,) (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res;
  PREBACKEND;
  BACKEND(double, &res, *arg1, *arg2);
  POSTBACKEND;
  Long *c = (Long*)(&res);
  return *c;
}


static VG_REGPARM(2) Long FCTCONVNAME(64F,) (Long a, Long b) {
  double *arg1 = (double*)(&a);
  float arg1f=*arg1;
  double *arg2 = (double*)(&b);
  float arg2f=*arg2;
  double res;
  float resf;
  PREBACKEND;
  BACKEND(float, &resf, arg1f, arg2f);
  POSTBACKEND;
  res=resf;
  Long *c = (Long*)(&res);
  return *c;
}


static VG_REGPARM(0) void  FCTNAME(64F,LLO) (void){
  double res;
  PREBACKEND;
  BACKEND(double, &res, arg1CopyDouble[0], arg2CopyDouble[0]);
  POSTBACKEND;
  arg1CopyDouble[0]=res;
}

static VG_REGPARM(0) void  FCTCONVNAME(64F,LLO) (void){
  float resf;
  float arg1f=arg1CopyDouble[0];
  float arg2f=arg2CopyDouble[0];
  PREBACKEND;
  BACKEND(float, &resf, arg1f, arg2f);
  POSTBACKEND;
  arg1CopyDouble[0]=resf;
}


static VG_REGPARM(1) void FCTNAME(64Fx2,)(/*OUT*/V128* output){
  const double* arg1=arg1CopyDouble;
  const double* arg2=arg2CopyDouble;
  double* res=(double*) output;
  PREBACKEND;
  BACKEND(double, res, arg1[0], arg2[0]);
  BACKEND(double, res+1, arg1[1], arg2[1]);
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTCONVNAME(64Fx2,)(/*OUT*/V128* output){
  const double* arg1=arg1CopyDouble;
  float arg1f[2]={arg1[0],arg1[1]};
  const double* arg2=arg2CopyDouble;
  float arg2f[2]={arg2[0],arg2[1]};
  double* res=(double*) output;
  float resf[2];
  PREBACKEND;
  BACKEND(float, resf, arg1f[0], arg2f[0]);
  BACKEND(float, resf+1, arg1f[1], arg2f[1]);
  POSTBACKEND;
  res[0]=resf[0];
  res[1]=resf[1];
}


static VG_REGPARM(1) void FCTNAME(64Fx4,) (/*OUT*/V256* output){
  double* res=(double*) output;
  PREBACKEND;
  for(int i=0; i<4; i++){
     BACKEND(double, res+i, arg1CopyDouble[i], arg2CopyDouble[i]);
  }
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTCONVNAME(64Fx4,) (/*OUT*/V256* output){
   float arg1f[4]={arg1CopyDouble[0],arg1CopyDouble[1],arg1CopyDouble[2],arg1CopyDouble[3]};
   float arg2f[4]={arg2CopyDouble[0],arg2CopyDouble[1],arg2CopyDouble[2],arg2CopyDouble[3]};
   double* res=(double*) output;
   float resf[4];
  PREBACKEND;
  for(int i=0; i<4; i++){
     BACKEND(float, resf+i, arg1f[i], arg2f[i]);
  }
  POSTBACKEND;
  res[0]=resf[0];
  res[1]=resf[1];
  res[2]=resf[2];
  res[3]=resf[3];
}


static VG_REGPARM(2) Int FCTNAME(32F,) (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res;
  PREBACKEND;
  BACKEND(float, &res, *arg1, *arg2);
  POSTBACKEND;
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(0) void FCTNAME(32F,LLO) (void){
  float res;
  PREBACKEND;
  BACKEND(float, &res, arg1CopyFloat[0], arg2CopyFloat[0]);
  POSTBACKEND;
  arg1CopyFloat[0]=res;
}

static VG_REGPARM(1) void FCTNAME(32Fx8,) (/*OUT*/V256* output){
  float* res=(float*) output;
  float* arg1=arg1CopyFloat;
  float* arg2=arg2CopyFloat;
  PREBACKEND;
  for(int i=0; i<8; i++){
     BACKEND(float, res+i, arg1[i], arg2[i]);
  }
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTNAME(32Fx4,) (/*OUT*/V128* output){
  float* res=(float*) output;
  const float* arg1=arg1CopyFloat;
  const float* arg2=arg2CopyFloat;
  PREBACKEND;
  for(int i=0; i<4;i++){
     BACKEND(float, res+i, arg1[i], arg2[i]);
  }
  POSTBACKEND;
}
