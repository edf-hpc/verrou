


    switch (op) {
      // Addition
      // - Double precision
    case Iop_AddF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr, bcNameWithCC(add64F), VR_OP_ADD, VR_PREC_DBL, VR_VEC_SCAL, countOnly);

    case Iop_Add64F0x2: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr, bcNameWithCC(add64F), VR_OP_ADD, VR_PREC_DBL, VR_VEC_LLO, countOnly);

    case Iop_Add64Fx2: // 128b vector, 2 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr, bcNameWithCC(add64Fx2), VR_OP_ADD, VR_PREC_DBL, VR_VEC_FULL2, countOnly);

    case Iop_AddF32: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr, bcNameWithCC(add32F), VR_OP_ADD, VR_PREC_FLT, VR_VEC_SCAL, countOnly);

    case Iop_Add32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr, bcNameWithCC(add32F), VR_OP_ADD, VR_PREC_FLT, VR_VEC_LLO, countOnly);

    case Iop_Add32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr, bcNameWithCC(add32Fx4), VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL4, countOnly);

    case Iop_Add64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr, bcNameWithCC(add64Fx4), VR_OP_ADD, VR_PREC_DBL, VR_VEC_FULL4, countOnly);

    case Iop_Add32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr, bcNameWithCC(add32Fx8), VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL8, countOnly);


      // Subtraction

      // - Double precision
    case Iop_SubF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr, bcNameWithCC(sub64F), VR_OP_SUB, VR_PREC_DBL, VR_VEC_SCAL, countOnly);

    case Iop_Sub64F0x2: // 128b vector, lowest-lane only
      return vr_replaceBinFpOpLLO (sb, stmt, expr, bcNameWithCC(sub64F), VR_OP_SUB, VR_PREC_DBL, VR_VEC_LLO, countOnly);

    case Iop_Sub64Fx2:
      return vr_replaceBinFullSSE (sb, stmt, expr, bcNameWithCC(sub64Fx2), VR_OP_SUB, VR_PREC_DBL, VR_VEC_FULL2, countOnly);

    case Iop_SubF32: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr, bcNameWithCC(sub32F), VR_OP_SUB, VR_PREC_FLT, VR_VEC_SCAL, countOnly);

    case Iop_Sub32F0x4: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr, bcNameWithCC(sub32F), VR_OP_SUB, VR_PREC_FLT, VR_VEC_LLO, countOnly);

    case Iop_Sub32Fx4: // 128b vector, 4 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr, bcNameWithCC(sub32Fx4), VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL4, countOnly);

    case Iop_Sub64Fx4: //AVX double
      return vr_replaceBinFullAVX(sb, stmt, expr, bcNameWithCC(sub64Fx4), VR_OP_SUB, VR_PREC_DBL, VR_VEC_FULL4, countOnly);

    case Iop_Sub32Fx8: //AVX Float
      return vr_replaceBinFullAVX(sb, stmt, expr, bcNameWithCC(sub32Fx8), VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL8, countOnly);

      // Multiplication

      // - Double precision
    case Iop_MulF64: // Scalar
      return vr_replaceBinFpOpScal (sb, stmt, expr, bcName(mul64F), VR_OP_MUL, VR_PREC_DBL, VR_VEC_SCAL, countOnly);

    case Iop_Mul64F0x2: // 128b vector, lowest-lane-only
      return vr_replaceBinFpOpLLO (sb, stmt, expr, bcName(mul64F), VR_OP_MUL, VR_PREC_DBL, VR_VEC_LLO, countOnly);

    case Iop_Mul64Fx2: // 128b vector, 2 lanes
      return vr_replaceBinFullSSE (sb, stmt, expr, bcName(mul64Fx2), VR_OP_MUL, VR_PREC_DBL, VR_VEC_FULL2, countOnly);

    case Iop_MulF32: // Scalar
       return vr_replaceBinFpOpScal (sb, stmt, expr, bcName(mul32F), VR_OP_MUL, VR_PREC_FLT, VR_VEC_SCAL, countOnly);

    case Iop_Mul32F0x4: // 128b vector, lowest-lane-only
       return vr_replaceBinFpOpLLO (sb, stmt, expr, bcName(mul32F), VR_OP_MUL, VR_PREC_FLT, VR_VEC_LLO, countOnly);

    case Iop_Mul32Fx4: // 128b vector, 4 lanes
       return vr_replaceBinFullSSE (sb, stmt, expr, bcName(mul32Fx4), VR_OP_MUL, VR_PREC_FLT, VR_VEC_FULL4, countOnly);

    case Iop_Mul64Fx4: //AVX double
       return vr_replaceBinFullAVX(sb, stmt, expr, bcName(mul64Fx4), VR_OP_MUL, VR_PREC_DBL, VR_VEC_FULL4, countOnly);

    case Iop_Mul32Fx8: //AVX Float
       return vr_replaceBinFullAVX(sb, stmt, expr, bcName(mul32Fx8), VR_OP_MUL, VR_PREC_FLT, VR_VEC_FULL8, countOnly);

    case Iop_DivF32:
       return vr_replaceBinFpOpScal (sb, stmt, expr, bcName(div32F), VR_OP_DIV, VR_PREC_FLT, VR_VEC_SCAL, countOnly);

    case Iop_Div32F0x4: // 128b vector, lowest-lane-only
       return vr_replaceBinFpOpLLO (sb, stmt, expr, bcName(div32F), VR_OP_DIV, VR_PREC_FLT, VR_VEC_LLO, countOnly);

    case Iop_Div32Fx4: // 128b vector, 4 lanes
       return vr_replaceBinFullSSE (sb, stmt, expr, bcName(div32Fx4), VR_OP_DIV, VR_PREC_FLT, VR_VEC_FULL4, countOnly);

    case Iop_DivF64: // Scalar
       return vr_replaceBinFpOpScal (sb, stmt, expr, bcName(div64F), VR_OP_DIV, VR_PREC_DBL, VR_VEC_SCAL, countOnly);

    case Iop_Div64F0x2: // 128b vector, lowest-lane-only
       return vr_replaceBinFpOpLLO (sb, stmt, expr, bcName(div64F), VR_OP_DIV, VR_PREC_DBL, VR_VEC_LLO, countOnly);

    case Iop_Div64Fx2: // 128b vector, 2 lanes
       return vr_replaceBinFullSSE(sb, stmt, expr, bcName(div64Fx2), VR_OP_DIV, VR_PREC_DBL, VR_VEC_FULL2, countOnly);

    case Iop_Div64Fx4: //AVX double
       return vr_replaceBinFullAVX(sb, stmt, expr, bcName(div64Fx4), VR_OP_DIV, VR_PREC_DBL, VR_VEC_FULL4, countOnly);

    case Iop_Div32Fx8: //AVX Float
       return vr_replaceBinFullAVX(sb, stmt, expr, bcName(div32Fx8), VR_OP_DIV, VR_PREC_FLT, VR_VEC_FULL8, countOnly);



    case Iop_MAddF32:
#ifndef IGNOREFMA
       return vr_replaceFMA (sb, stmt, expr, bcNameWithCC(madd32F), VR_OP_MADD, VR_PREC_FLT, countOnly);
#else
       vr_countOp (sb, VR_OP_MADD, VR_PREC_FLT, VR_VEC_UNK,False);
       addStmtToIRSB (sb, stmt);
       break;
#endif
    case Iop_MSubF32:
#ifndef IGNOREFMA
          return vr_replaceFMA (sb, stmt, expr, bcNameWithCC(msub32F), VR_OP_MSUB, VR_PREC_FLT, countOnly);
#else
       vr_countOp (sb, VR_OP_MSUB, VR_PREC_FLT, VR_VEC_UNK,False);
       addStmtToIRSB (sb, stmt);
       break;
#endif
    case Iop_MAddF64:
#ifndef IGNOREFMA
          return vr_replaceFMA (sb, stmt, expr, bcNameWithCC(madd64F), VR_OP_MADD, VR_PREC_DBL, countOnly);
#else
       vr_countOp (sb, VR_OP_MADD, VR_PREC_DBL, VR_VEC_UNK,False);
       addStmtToIRSB (sb, stmt);
       break;
#endif
    case Iop_MSubF64:
#ifndef IGNOREFMA
          return vr_replaceFMA (sb, stmt, expr, bcNameWithCC(msub64F), VR_OP_MSUB,  VR_PREC_DBL, countOnly);
#else
       vr_countOp (sb, VR_OP_MSUB, VR_PREC_DBL, VR_VEC_UNK,False);
       addStmtToIRSB (sb, stmt);
       break;
#endif
      //   Other FP operations
    case Iop_Add32Fx2:
      vr_countOp (sb, VR_OP_ADD, VR_PREC_FLT, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;


    case Iop_Sub32Fx2:
      vr_countOp (sb, VR_OP_SUB, VR_PREC_FLT, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_CmpF64:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_CmpF32:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F32toF64:  /*                       F32 -> F64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_FLT_TO_DBL, VR_VEC_UNK,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toF32:
#ifndef IGNORECAST
       return vr_replaceCast (sb, stmt, expr, bcName(cast64FTo32F), VR_OP_CONV, VR_PREC_DBL_TO_FLT, countOnly);
#else
       vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_FLT, VR_VEC_UNK,False);
       addStmtToIRSB (sb, stmt);
#endif
       break;

    case Iop_F64toI64S: /* IRRoundingMode(I32) x F64 -> signed I64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_INT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI64U: /* IRRoundingMode(I32) x F64 -> unsigned I64 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_INT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI32S: /* IRRoundingMode(I32) x F64 -> signed I32 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_SHT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_F64toI32U: /* IRRoundingMode(I32) x F64 -> unsigned I32 */
      vr_countOp (sb, VR_OP_CONV, VR_PREC_DBL_TO_SHT, VR_VEC_SCAL,False);
      addStmtToIRSB (sb, stmt);
      break;

      /******/
    case Iop_Max32Fx4:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max32F0x4:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max64Fx2:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Max64F0x2:
      vr_countOp (sb, VR_OP_MAX, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;



    case Iop_Min32Fx4:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min32F0x4:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min64Fx2:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_Min64F0x2:
      vr_countOp (sb, VR_OP_MIN, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;


    case Iop_CmpEQ64Fx2: case Iop_CmpLT64Fx2:
    case Iop_CmpLE64Fx2: case Iop_CmpUN64Fx2:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_FULL2,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ64F0x2: case Iop_CmpLT64F0x2:
    case Iop_CmpLE64F0x2: case Iop_CmpUN64F0x2:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_DBL, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ32Fx4: case Iop_CmpLT32Fx4:
    case Iop_CmpLE32Fx4: case Iop_CmpUN32Fx4:
    case Iop_CmpGT32Fx4: case Iop_CmpGE32Fx4:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_FULL4,False);
      addStmtToIRSB (sb, stmt);
      break;
    case Iop_CmpEQ32F0x4: case Iop_CmpLT32F0x4:
    case Iop_CmpLE32F0x4: case Iop_CmpUN32F0x4:
      vr_countOp (sb, VR_OP_CMP, VR_PREC_FLT, VR_VEC_LLO,False);
      addStmtToIRSB (sb, stmt);
      break;

    case Iop_ReinterpF64asI64:
    case Iop_ReinterpI64asF64:
    case Iop_ReinterpF32asI32:
    case Iop_ReinterpI32asF32:
    case Iop_NegF64:
    case Iop_AbsF64:
    case Iop_NegF32:
    case Iop_AbsF32:
    case Iop_Abs64Fx2:
    case Iop_Neg64Fx2:
      //ignored : not counted and not instrumented
      addStmtToIRSB (sb, stmt);
      break;

      //operation with 64bit register with 32bit rounding
    case Iop_AddF64r32:
    case Iop_SubF64r32:
    case Iop_MulF64r32:
    case Iop_DivF64r32:
    case Iop_MAddF64r32:
    case Iop_MSubF64r32:

      //operation with 128bit
    case Iop_AddF128:
    case Iop_SubF128:
    case Iop_MulF128:
    case Iop_DivF128:

    case Iop_SqrtF128:
    case Iop_SqrtF64:
    case Iop_SqrtF32:
    case Iop_Sqrt32Fx4:
    case Iop_Sqrt64Fx2:
    case  Iop_AtanF64:       /* FPATAN,  arctan(arg1/arg2)       */
    case  Iop_Yl2xF64:       /* FYL2X,   arg1 * log2(arg2)       */
    case  Iop_Yl2xp1F64:     /* FYL2XP1, arg1 * log2(arg2+1.0)   */
    case  Iop_PRemF64:       /* FPREM,   non-IEEE remainder(arg1/arg2)    */
    case  Iop_PRemC3210F64:  /* C3210 flags resulting from FPREM: :: I32 */
    case  Iop_PRem1F64:      /* FPREM1,  IEEE remainder(arg1/arg2)    */
    case  Iop_PRem1C3210F64: /* C3210 flags resulting from FPREM1, :: I32 */
    case  Iop_ScaleF64:      /* FSCALE,  arg1 * (2^RoundTowardsZero(arg2)) */
    case  Iop_SinF64:    /* FSIN */
    case  Iop_CosF64:    /* FCOS */
    case  Iop_TanF64:    /* FTAN */
    case  Iop_2xm1F64:   /* (2^arg - 1.0) */

    case Iop_RSqrtEst5GoodF64: /* reciprocal square root estimate, 5 good bits */


    case Iop_Log2_64Fx2:
    case Iop_Scale2_64Fx2:
    case Iop_RecipEst64Fx2:    // unary
    case Iop_RecipStep64Fx2:   // binary
    case Iop_RSqrtEst64Fx2:   // unary
    case Iop_RSqrtStep64Fx2:   // binary

    case Iop_RecipStep32Fx4:
    case Iop_RSqrtEst32Fx4:
    case Iop_RSqrtStep32Fx4:
    case Iop_RecipEst32F0x4:
    case Iop_Sqrt32F0x4:
    case Iop_RSqrtEst32F0x4:
    case Iop_Scale2_32Fx4:
    case Iop_Log2_32Fx4:
    case Iop_Exp2_32Fx4:
      /*AVX*/
    case Iop_Sqrt16Fx8:
    case Iop_Sqrt32Fx8:
    case Iop_Sqrt64Fx4:
    case Iop_RSqrtEst32Fx8:
    case Iop_RecipEst32Fx8:

    case Iop_RoundF64toF64_NEAREST: /* frin */
    case Iop_RoundF64toF64_NegINF:  /* frim */
    case Iop_RoundF64toF64_PosINF:  /* frip */
    case Iop_RoundF64toF64_ZERO:    /* friz */

    case Iop_F32toF16x4:
    case Iop_F16toF32x4:
    case Iop_F16toF64x2:

    case Iop_F128toF64:  /* IRRoundingMode(I32) x F128 -> F64         */
    case Iop_F128toF32:  /* IRRoundingMode(I32) x F128 -> F32         */
    case Iop_F64toI16S: /* IRRoundingMode(I32) x F64 -> signed I16 */

    case Iop_CmpF128:

    case Iop_PwMax32Fx4: case Iop_PwMin32Fx4:
      vr_maybe_record_ErrorOp (VR_ERROR_UNCOUNTED, op);
      addStmtToIRSB (sb, stmt);
      break;

    default:
      addStmtToIRSB (sb, stmt);
      break;
    }
    return False;
