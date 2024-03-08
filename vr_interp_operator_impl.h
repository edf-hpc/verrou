
/*Tools for copy : workaround to avoid limitation of 6args*/

#ifndef INTERFLOP_VECTO

static double arg1CopySSEDouble[2];
static double arg2CopySSEDouble[2];
static float arg1CopySSEFloat[4];
static float arg2CopySSEFloat[4];

static double arg1CopyAvxDouble[4];
static double arg2CopyAvxDouble[4];
static float arg1CopyAvxFloat[8];
static float arg2CopyAvxFloat[8];

#else
static doublex4 arg1CopyAvxDouble;
static doublex4 arg2CopyAvxDouble;

static VG_REGPARM(3) void vr_AvxDoubleCopyFirstArg (ULong a0, ULong a1, ULong a2,ULong a3) {
  arg1CopyAvxDouble= (doublex4){*((double*)(&a0)),*((double*)(&a1)),*((double*)(&a2)),*((double*)(&a3))};
}

static floatx8 arg1CopyAvxFloat;
static floatx8 arg2CopyAvxFloat;

static VG_REGPARM(3) void vr_AvxFloatCopyFirstArg (ULong a0, ULong a1, ULong a2,ULong a3) {
  V256* reg1=(V256*)(&arg1CopyAvxFloat) ;
  reg1->w64[0]=a0;
  reg1->w64[1]=a1;
  reg1->w64[2]=a2;
  reg1->w64[3]=a3;
  arg1CopyAvxFloat=*reg1;
}

#endif
