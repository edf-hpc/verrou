

static VG_REGPARM(0) void  FCTNAME(64F,) (void){
  double res; 
  PREBACKEND;
  BACKENDFUNC(double)(arg1CopyDouble[0], arg2CopyDouble[0], &res, CONTEXT);
  POSTBACKEND;
  arg1CopyDouble[0]=res;
}

static VG_REGPARM(1) void FCTNAME(64Fx2,)(/*OUT*/V128* output){
  const double* arg1=arg1CopyDouble;
  const double* arg2=arg2CopyDouble;
  double* res=(double*) output;
  PREBACKEND;
  BACKENDFUNC(double)(arg1[0], arg2[0], res, CONTEXT);
  BACKENDFUNC(double)(arg1[1], arg2[1], res+1, CONTEXT);
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTNAME(64Fx4,) (/*OUT*/V256* output){
  double* res=(double*) output;
  PREBACKEND;
  for(int i=0; i<4; i++){
     BACKENDFUNC(double)(arg1CopyDouble[i], arg2CopyDouble[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}

static VG_REGPARM(0) void FCTNAME(32F,) (void){
  float res;
  PREBACKEND;
  BACKENDFUNC(float)(arg1CopyFloat[0], arg2CopyFloat[0], &res, CONTEXT);
  POSTBACKEND;
  arg1CopyFloat[0]=res;
}

static VG_REGPARM(1) void FCTNAME(32Fx8,) (/*OUT*/V256* output){
  float* res=(float*) output;
  float* arg1=arg1CopyFloat;
  float* arg2=arg2CopyFloat;
  PREBACKEND;
  for(int i=0; i<8; i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}

static VG_REGPARM(1) void FCTNAME(32Fx4,) (/*OUT*/V128* output){
  float* res=(float*) output;
  const float* arg1=arg1CopyFloat;
  const float* arg2=arg2CopyFloat;
  PREBACKEND;
  for(int i=0; i<4;i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
  POSTBACKEND;
}


