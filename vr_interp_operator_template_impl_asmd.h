

#ifdef ADDINTERP
#define FCTNAME(TYPEVAL,OPTION) vr_Add##TYPEVAL##OPTION
#define BACKENDFUNC(TYPEC) (interflop_verrou_add_##TYPEC)
#endif
#undef ADDINTERP

#ifdef SUBINTERP
#define FCTNAME(TYPEVAL,OPTION) vr_Sub##TYPEVAL##OPTION
#define BACKENDFUNC(TYPEC) (interflop_verrou_sub_##TYPEC)
#endif
#undef SUBINTERP

#ifdef MULINTERP
#define FCTNAME(TYPEVAL,OPTION) vr_Mul##TYPEVAL##OPTION
#define BACKENDFUNC(TYPEC) (interflop_verrou_mul_##TYPEC)
#endif
#undef MULINTERP


#ifdef DIVINTERP
#define FCTNAME(TYPEVAL,OPTION) vr_Div##TYPEVAL##OPTION
#define BACKENDFUNC(TYPEC) (interflop_verrou_div_##TYPEC)
#endif
#undef DIVINTERP





static VG_REGPARM(2) Long FCTNAME(64F,) (Long a, Long b) {
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double res;
  (BACKENDFUNC(double))(*arg1, *arg2, &res, backend_context);
  Long *c = (Long*)(&res);
  return *c;
}



static VG_REGPARM(3) void FCTNAME(64Fx2,)(/*OUT*/V128* output, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
  double arg1[2] = {*((double*)(&aLo)),*((double*)(&aHi))} ;
  double arg2[2] = {*((double*)(&bLo)),*((double*)(&bHi))} ;
  double* res=(double*) output;
  //a terme appel a doublex2
  (BACKENDFUNC(double))(arg1[0], arg2[0], res, backend_context);
  (BACKENDFUNC(double))(arg1[1], arg2[1], res+1, backend_context);
}


/*static VG_REGPARM(3) void FCTNAME(64Fx2,Fallback)(V128* output, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
  double arg1[2] = {*((double*)(&aLo)),*((double*)(&aHi))} ;
  double arg2[2] = {*((double*)(&bLo)),*((double*)(&bHi))} ;
  double* res=(double*) output;
  (BACKENDFUNC(double))(arg1[0], arg2[0], res, backend_context);
  (BACKENDFUNC(double))(arg1[1], arg2[1], res+1, backend_context);
  }*/

/*
static VG_REGPARM(3) void FCTNAME(64Fx4,AllArgs) (V256* output,
				       ULong a0, ULong a1, ULong a2,ULong a3,
				       ULong b0, ULong b1, ULong b2,ULong b3) {
  double arg1[4] = {*((double*)(&a0)),*((double*)(&a1)), *((double*)(&a2)),*((double*)(&a3))} ;
  double arg2[4] = {*((double*)(&b0)),*((double*)(&b1)), *((double*)(&b2)),*((double*)(&b3))} ;
  double* res=(double*) output;
  for(int i=0; i<4; i++){
    (BACKENDFUNC(double))(arg1[i], arg2[i], res+i, backend_context);
  }
}
*/

static VG_REGPARM(3) void FCTNAME(64Fx4,) (/*OUT*/V256* output,
							 ULong b0, ULong b1, ULong b2,ULong b3) {
  double arg2[4] = {*((double*)(&b0)),*((double*)(&b1)), *((double*)(&b2)),*((double*)(&b3))} ;
  double* res=(double*) output;
  //a terme appel a doublex4
  for(int i=0; i<4; i++){
    (BACKENDFUNC(double)) (arg1CopyAvxDouble[i], arg2[i], res+i, backend_context);
  }
}

/*static VG_REGPARM(3) void FCTNAME(64Fx4,Fallback) (V256* output,
						   ULong b0, ULong b1, ULong b2,ULong b3) {
  double arg2[4] = {*((double*)(&b0)),*((double*)(&b1)), *((double*)(&b2)),*((double*)(&b3))} ;
  double* res=(double*) output;
  for(int i=0; i<4; i++){
    (BACKENDFUNC(double)) (arg1CopyAvxDouble[i], arg2[i], res+i, backend_context);
  }
  }*/


static VG_REGPARM(2) Int FCTNAME(32F,) (Long a, Long b) {
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float res;
  (BACKENDFUNC(float))(*arg1, *arg2, &res, backend_context);
  Int *c = (Int*)(&res);
  return *c;
}

static VG_REGPARM(3) void FCTNAME(32Fx8,) (/*OUT*/V256* output,
						ULong b0, ULong b1, ULong b2,ULong b3) {

  V256 reg2;   reg2.w64[0]=b0;   reg2.w64[1]=b1;   reg2.w64[2]=b2;   reg2.w64[3]=b3;
  float* res=(float*) output;
  float* arg1=arg1CopyAvxFloat;
  float* arg2=(float*) &reg2;
  for(int i=0; i<8; i++){
    (BACKENDFUNC(float))(arg1[i], arg2[i], res+i, backend_context);
  }
}

/*static VG_REGPARM(3) void FCTNAME(32Fx8,FallBack) (V256* output,
						   ULong b0, ULong b1, ULong b2,ULong b3) {

  V256 reg2;   reg2.w64[0]=b0;   reg2.w64[1]=b1;   reg2.w64[2]=b2;   reg2.w64[3]=b3;
  float* res=(float*) output;
  float* arg1=arg1CopyAvxFloat;
  float* arg2=(float*) &reg2;
  for(int i=0; i<8; i++){
    (BACKENDFUNC(float))(arg1[i], arg2[i], res+i, backend_context);
  }
  }*/


static VG_REGPARM(3) void FCTNAME(32Fx4,) (/*OUT*/V128* output, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
  V128 reg1; reg1.w64[0]=aLo; reg1.w64[1]=aHi;
  V128 reg2; reg2.w64[0]=bLo; reg2.w64[1]=bHi;

  float* res=(float*) output;
  float* arg1=(float*) &reg1;
  float* arg2=(float*) &reg2;

  for(int i=0; i<4;i++){
    (BACKENDFUNC(float))(arg1[i], arg2[i], res+i, backend_context);
  }
}

/*static VG_REGPARM(3) void FCTNAME(32Fx4,Fallback) (V128* output, ULong aHi, ULong aLo, ULong bHi,ULong bLo) {
  V128 reg1; reg1.w64[0]=aLo; reg1.w64[1]=aHi;
  V128 reg2; reg2.w64[0]=bLo; reg2.w64[1]=bHi;

  float* res=(float*) output;
  float* arg1=(float*) &reg1;
  float* arg2=(float*) &reg2;

  for(int i=0; i<4;i++){
    (BACKENDFUNC(float))(arg1[i], arg2[i], res+i, backend_context);
  }
  }*/





#undef FCTNAME
#undef BACKENDFUNC
