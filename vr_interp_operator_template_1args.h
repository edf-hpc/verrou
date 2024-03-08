

static VG_REGPARM(0) void FCTNAME(64F,) (void) {
  double res;
  BACKENDFUNC(double)(arg1CopySSEDouble[0], &res, CONTEXT);
  arg1CopySSEDouble[0]=res;
}

static VG_REGPARM(3) void FCTNAME(64Fx2,)(/*OUT*/V128* output, ULong aHi, ULong aLo) {
  double arg1[2] = {*((double*)(&aLo)),*((double*)(&aHi))} ;
  double* res=(double*) output;
  BACKENDFUNC(double)(arg1[0], res, CONTEXT);
  BACKENDFUNC(double)(arg1[1], res+1, CONTEXT);
}

static VG_REGPARM(3) void FCTNAME(64Fx4,) (/*OUT*/V256* output,
                                           ULong a0, ULong a1, ULong a2,ULong a3) {

  double arg1[4] = {*((double*)(&a0)),*((double*)(&a1)), *((double*)(&a2)),*((double*)(&a3))} ;
  double* res=(double*) output;
  for(int i=0; i<4; i++){
     BACKENDFUNC(double)(arg1[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(0) void FCTNAME(32F,) (void) {
  float res;
  BACKENDFUNC(float)(arg1CopySSEFloat[0], &res, CONTEXT);
  arg1CopySSEFloat[0]=res;
}

static VG_REGPARM(3) void FCTNAME(32Fx8,) (/*OUT*/V256* output,
					   ULong a0, ULong a1, ULong a2,ULong a3) {
  V256 reg1;   reg1.w64[0]=a0;   reg1.w64[1]=a1;   reg1.w64[2]=a2;   reg1.w64[3]=a3;
  float* res=(float*) output;
  float* arg1=(float*) &reg1;
  for(int i=0; i<8; i++){
     BACKENDFUNC(float)(arg1[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(3) void FCTNAME(32Fx4,) (/*OUT*/V128* output, ULong aHi, ULong aLo) {
  V128 reg1; reg1.w64[0]=aLo; reg1.w64[1]=aHi;

  float* res=(float*) output;
  float* arg1=(float*) &reg1;

  for(int i=0; i<4;i++){
     BACKENDFUNC(float)(arg1[i], res+i, CONTEXT);
  }
}


