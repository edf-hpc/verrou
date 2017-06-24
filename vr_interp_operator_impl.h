
/*Tools for copy : workaround to avoid limitation of 6args*/
static double arg1CopyAvxDouble[4];
static VG_REGPARM(3) void vr_AvxDoubleCopyFirstArg (ULong a0, ULong a1, ULong a2,ULong a3) {
  arg1CopyAvxDouble[0] =*((double*)(&a0));
  arg1CopyAvxDouble[1] =*((double*)(&a1));
  arg1CopyAvxDouble[2] =*((double*)(&a2));
  arg1CopyAvxDouble[3] =*((double*)(&a3));    
}

static float arg1CopyAvxFloat[8];
static VG_REGPARM(3) void vr_AvxFloatCopyFirstArg (ULong a0, ULong a1, ULong a2,ULong a3) {
  V256* reg1=(V256*)(&arg1CopyAvxFloat) ;
  reg1->w64[0]=a0;
  reg1->w64[1]=a1;
  reg1->w64[2]=a2;
  reg1->w64[3]=a3;
}



#define ADDINTERP
#include "vr_interp_operator_template_impl_asmd.h" 

#define SUBINTERP
#include "vr_interp_operator_template_impl_asmd.h" 

#define MULINTERP
#include "vr_interp_operator_template_impl_asmd.h" 

#define DIVINTERP
#include "vr_interp_operator_template_impl_asmd.h" 








//FMA Operator
static VG_REGPARM(3) Long vr_MAdd64F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res;
  (*backend.interflop_madd_double)(*arg1, *arg2, *arg3, &res, backend_context);
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int vr_MAdd32F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res;
  (*backend.interflop_madd_float) (*arg1, *arg2, *arg3, &res,backend_context);
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}




static VG_REGPARM(3) Long vr_MSub64F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  double *arg1 = (double*)(&a);
  double *arg2 = (double*)(&b);
  double *arg3 = (double*)(&c);
  double res;
  (*backend.interflop_madd_double) (*arg1, *arg2, -*arg3,&res, backend_context);
#else
  double res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Long *d = (Long*)(&res);
  return *d;
}

static VG_REGPARM(3) Int vr_MSub32F (Long a, Long b, Long c) {
#ifdef USE_VERROU_FMA
  float *arg1 = (float*)(&a);
  float *arg2 = (float*)(&b);
  float *arg3 = (float*)(&c);
  float res;
  (*backend.interflop_madd_float) (*arg1, *arg2, -*arg3, &res, backend_context);
#else
  float res=0.;
  VG_(tool_panic) ( "Verrou needs to be compiled with FMA support \n");
#endif
  Int *d = (Int*)(&res);
  return *d;
}
