

static VG_REGPARM(0) Long FCTNAME(64F,) (void){ //Long a, Long b) {
  //  double *arg1 = (double*)(&a);
  //  double *arg2 = (double*)(&b);
  double res;
  BACKENDFUNC(double)(arg1CopySSEDouble[0], arg2CopySSEDouble[0], &res, CONTEXT);
  Long *c = (Long*)(&res);
  return *c;
}

static VG_REGPARM(0) void FCTNAME(64Fx2,)(/*OUT*/V128* output){//, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
//  double arg1[2] = {*((double*)(&aLo)),*((double*)(&aHi))} ;
//  double arg2[2] = {*((double*)(&bLo)),*((double*)(&bHi))} ;
  const double* arg1=arg1CopySSEDouble;
  const double* arg2=arg2CopySSEDouble;
  double* res=(double*) output;
  BACKENDFUNC(double)(arg1[0], arg2[0], res, CONTEXT);
  BACKENDFUNC(double)(arg1[1], arg2[1], res+1, CONTEXT);
}

static VG_REGPARM(0) void FCTNAME(64Fx4,) (/*OUT*/V256* output){
//                                           ULong b0, ULong b1, ULong b2,ULong b3) {

//  double arg2[4] = {*((double*)(&b0)),*((double*)(&b1)), *((double*)(&b2)),*((double*)(&b3))} ;
  double* res=(double*) output;
  for(int i=0; i<4; i++){
     BACKENDFUNC(double)(arg1CopyAvxDouble[i], arg2CopyAvxDouble[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(0) Int FCTNAME(32F,) (void){ //Long a, Long b) {
  //  float *arg1 = (float*)(&a);
  //  float *arg2 = (float*)(&b);
  float res;
  BACKENDFUNC(float)(arg1CopySSEFloat[0], arg2CopySSEFloat[0], &res, CONTEXT);
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(0) void FCTNAME(32Fx8,) (/*OUT*/V256* output){
//					   ULong b0, ULong b1, ULong b2,ULong b3) {
//  V256 reg2;   reg2.w64[0]=b0;   reg2.w64[1]=b1;   reg2.w64[2]=b2;   reg2.w64[3]=b3;
  float* res=(float*) output;
  float* arg1=arg1CopyAvxFloat;
  float* arg2=arg2CopyAvxFloat;
//  float* arg2=(float*) &reg2;
  for(int i=0; i<8; i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
}

static VG_REGPARM(0) void FCTNAME(32Fx4,) (/*OUT*/V128* output){//, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
/*  V128 reg1; reg1.w64[0]=aLo; reg1.w64[1]=aHi;
    V128 reg2; reg2.w64[0]=bLo; reg2.w64[1]=bHi;*/

  float* res=(float*) output;
//  float* arg1=(float*) &reg1;
//  float* arg2=(float*) &reg2;

  const float* arg1=arg1CopySSEFloat;
  const float* arg2=arg2CopySSEFloat;
  for(int i=0; i<4;i++){
     BACKENDFUNC(float)(arg1[i], arg2[i], res+i, CONTEXT);
  }
}


