

static VG_REGPARM(0) void  FCTNAME(64F,) (void){
  double res;
  BACKENDFUNC(double)(arg1CopySSEDouble[0], arg2CopySSEDouble[0], &res, CONTEXT);
  arg1CopySSEDouble[0]=res;
}

static VG_REGPARM(1) void FCTNAME(64Fx2,)(/*OUT*/V128* output){
  const double* arg1=arg1CopySSEDouble;
  const double* arg2=arg2CopySSEDouble;
  double* res=(double*) output;
  BACKENDFUNC(double)(arg1[0], arg2[0], res, CONTEXT);
  BACKENDFUNC(double)(arg1[1], arg2[1], res+1, CONTEXT);
}

static VG_REGPARM(1) void FCTNAME(64Fx4,) (/*OUT*/V256* output){
  double* res=(double*) output;
  for(int i=0; i<4; i++){
     BACKENDFUNC(double)(arg1CopyAvxDouble[i], arg2CopyAvxDouble[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(0) void FCTNAME(32F,) (void){
  float res;
  BACKENDFUNC(float)(arg1CopySSEFloat[0], arg2CopySSEFloat[0], &res, CONTEXT);
  arg1CopySSEFloat[0]=res;
}

static VG_REGPARM(1) void FCTNAME(32Fx8,) (/*OUT*/V256* output){
  float* res=(float*) output;
  float* arg1=arg1CopyAvxFloat;
  float* arg2=arg2CopyAvxFloat;
  for(int i=0; i<8; i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(1) void FCTNAME(32Fx4,) (/*OUT*/V128* output){
  float* res=(float*) output;

  const float* arg1=arg1CopySSEFloat;
  const float* arg2=arg2CopySSEFloat;
  for(int i=0; i<4;i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
}


