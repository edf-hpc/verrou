AM_PATH_PYTHON([3])

#--enable-verrou-fma
AC_CACHE_CHECK([verrou fma], vg_cv_verrou_fma,
  [AC_ARG_ENABLE(verrou-fma,
    [  --enable-verrou-fma          enables verrou to use fma],
    [vg_cv_verrou_fma=$enableval],
    [vg_cv_verrou_fma=yes])])

if test "$vg_cv_verrou_fma" = yes; then
  # Check for fmaintrin.h
  AC_LANG_PUSH(C++)
    CXXFLAGS="$safe_CXXFLAGS -march=native"
  AC_MSG_CHECKING([for fma instrinsics])
case "$ARCH_MAX-$VGCONF_OS" in
     amd64-linux)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include  <immintrin.h>
      #include <stdlib.h>
]], [[
       double a,b,c,d;
      __m128d ai, bi,ci,di;
      ai = _mm_load_sd(&a);
      bi = _mm_load_sd(&b);
      ci = _mm_load_sd(&c);
      di=_mm_fmadd_sd(ai,bi,ci);
      d=_mm_cvtsd_f64(di);
      return EXIT_SUCCESS;
    ]])],
    [
      AC_MSG_RESULT([yes])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with fmaintrin.h is required for --enable-verrou-fma ])
  ]) ;;
  arm64-linux)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  #include "arm_neon.h"
  #include <stdlib.h>
]], [[
  double a,b,c,d;
  float64x1_t ai, bi,ci,di;
  ai = vld1_dup_f64(&a);
  bi = vld1_dup_f64(&b);
  ci = vld1_dup_f64(&c);
  di=vfma_f64(ai,bi,ci);
  vst1_f64(&d, di);
      return EXIT_SUCCESS;
    ]])],
    [
      AC_MSG_RESULT([yes])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with arm_neon is required for --enable-verrou-fma ])
  ]) ;;
  *)
  vg_cv_verrou_fma=no
  ;;
  esac
  AC_LANG_POP(C++)
fi

AM_CONDITIONAL([HAVE_VERROU_FMA_INTRIN], test x$vg_cv_verrou_fma = xyes,[])
AC_SUBST(vg_cv_verrou_fma)


#--enable-verrou-sqrt
AC_CACHE_CHECK([verrou ], vg_cv_verrou_sqrt,
  [AC_ARG_ENABLE(verrou-sqrt,
    [  --enable-verrou-sqrt          enables verrou to use sqrt],
    [vg_cv_verrou_sqrt=$enableval],
    [vg_cv_verrou_sqrt=yes])])

if test "$vg_cv_verrou_sqrt" = yes; then
  # Check for sqrt intrinsics
  AC_LANG_PUSH(C++)
  AC_MSG_CHECKING([for sqrt intrinsic ])
case "$ARCH_MAX-$VGCONF_OS" in
     amd64-linux)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include  <immintrin.h>
      #include <stdlib.h>
]], [[
       double a=1.,d;
      __m128d ai, di;
      ai = _mm_load_sd(&a);
      di=_mm_sqrt_sd(ai,ai);
      d=_mm_cvtsd_f64(di);
      return EXIT_SUCCESS;
    ]])],
    [
      AC_MSG_RESULT([yes])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with sqrt intrinsics is required for --enable-verrou-sqrt ])
  ]);;
    arm64-linux)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  #include "arm_neon.h"
  #include <stdlib.h>
]], [[
  double a=1.2;
  double d=vrsqrted_f64(a);
      return EXIT_SUCCESS;
    ]])],
    [
      AC_MSG_RESULT([yes])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with arm_neon is required for --enable-verrou-sqrt ])
  ])
  ;;
  *)
  vg_cv_verrou_sqrt=no
  esac
  AC_LANG_POP(C++)
fi

AM_CONDITIONAL([HAVE_VERROU_SQRT_INTRIN], test x$vg_cv_verrou_sqrt = xyes,[])
AC_SUBST(vg_cv_verrou_sqrt)


#--enable-verrou-quadmath
AC_CACHE_CHECK([verrou quadmath], vg_cv_verrou_quadmath,
  [AC_ARG_ENABLE(verrou-quadmath,
    [  --enable-verrou-quadmath          enables verrou to use __float128 and quadmath],
    [vg_cv_verrou_quadmath=$enableval],
    [vg_cv_verrou_quadmath=yes])])

if test "$vg_cv_verrou_quadmath" = yes; then
  # Check for quadmath.h
  AC_LANG_PUSH(C++)
    CXXFLAGS="$safe_CXXFLAGS -lquadmath"
  AC_MSG_CHECKING([for quadmath])
case "$ARCH_MAX-$VGCONF_OS" in
     amd64-linux)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include  <quadmath.h>
      #include <stdlib.h>
]], [[
       __float128 a=0.1,b=0.1,c;
       c=a+b;
      return EXIT_SUCCESS;
    ]])],
    [
      AC_MSG_RESULT([yes])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with quadmath.h is required for --enable-verrou-quadmath ])
  ]) ;;
  arm64-linux)
     AC_MSG_RESULT([no])
     vg_cv_verrou_quadmath=no
  ;;
  *)
  vg_cv_verrou_quadmath=no
  ;;
  esac
  AC_LANG_POP(C++)
fi

AM_CONDITIONAL([HAVE_VERROU_QUADMATH], test x$vg_cv_verrou_quadmath = xyes,[])
AC_SUBST(vg_cv_verrou_quadmath)


#--enable-profiling for exact operation
AC_CACHE_CHECK([verrou profiling exact], vg_cv_verrou_profiling_exact,
  [AC_ARG_ENABLE(verrou-profiling-exact,
    [  --enable-verrou-profiling-exact          enables profiling of exact operation],
    [vg_cv_verrou_profiling_exact=$enableval],
    [vg_cv_verrou_profiling_exact=no])])

AM_CONDITIONAL([USE_PROFILING_EXACT], test x$vg_cv_verrou_profiling_exact = xyes,[])
AC_SUBST(vg_cv_verrou_profiling_exact)


#--enable-verrou-optim for exact operation
AC_CACHE_CHECK([verrou optim], vg_cv_verrou_optim,
  [AC_ARG_ENABLE(verrou-optim,
    [  --enable-verrou-optim          enables verrou optimisation (require lot of memory at compile time)],
    [vg_cv_verrou_optim=$enableval],
    [vg_cv_verrou_optim=yes])])

AM_CONDITIONAL([USE_VERROU_OPTIM], test x$vg_cv_verrou_optim = xyes,[])
AC_SUBST(vg_cv_verrou_optim)



#--enable-xoshiro
AC_CACHE_CHECK([verrou xoshiro prng], vg_cv_verrou_xoshiro,
  [AC_ARG_ENABLE(verrou-xoshiro,
    [  --enable-verrou-xoshiro          enables xoshiro prng for random and average rounding mode],
    [vg_cv_verrou_xoshiro=$enableval],
    [vg_cv_verrou_xoshiro=yes])])

AM_CONDITIONAL([USE_XOSHIRO], test x$vg_cv_verrou_xoshiro = xyes,[])
AC_SUBST(vg_cv_verrou_xoshiro)


#--enable-verrou-checknaninf
AC_CACHE_CHECK([verrou check nan inf], vg_cv_verrou_naninf,
  [AC_ARG_ENABLE(verrou-check-naninf,
    [  --enable-verrou-check-naninf          enables verrou check for nan and inf],
    [vg_cv_verrou_naninf=$enableval],
    [vg_cv_verrou_naninf=yes])])

AM_CONDITIONAL([USE_VERROU_NANINF], test x$vg_cv_verrou_naninf = xyes,[])
AC_SUBST(vg_cv_verrou_naninf)



AC_ARG_VAR(VERROU_NUM_AVG,[Number of AVG rounding per 64bit generated by mersenne twister or xoshiro])
AS_VAR_SET_IF([VERROU_NUM_AVG], [],[VERROU_NUM_AVG=1])

AS_CASE([$VERROU_NUM_AVG],
	[1],[],
	[2],[],
	[3],[],
	[4],[],
	[8],[],
	[16],[],
	[32],[],
	[64],[],
	[*],[AC_MSG_ERROR(["invalid VERROU_NUM_AVG", $VERROU_NUM_AVG])]
)



AC_CACHE_CHECK([verrou-det-hash], vg_cv_verrou_det_hash,
[
AC_ARG_WITH(
	[verrou-det-hash],
	[  --with-verrou-det-hash=hash_name	hash algorithm for random_[com]det and average_[com]det: dietzfelbinger,multiply_shift,[double_]tabulation,xxhash,mersenne_twister],
	[vg_cv_verrou_det_hash=$withval],
	[vg_cv_verrou_det_hash=xxhash]
)])

AS_CASE([$vg_cv_verrou_det_hash],
	[yes],[vg_cv_verrou_det_hash=double_tabulation],
	[dietzfelbinger],[echo "dietzfelbinger hash selected"],
	[multiply_shift],[echo "multiply_shift hash selected"],
	[tabulation],[echo "tabulation hash selected"],
	[double_tabulation],[echo "double_tabulation hash selected"],
	[xxhash],[echo "xxHash hash selected"],
	[mersenne_twister],[echo "mersenne_twister hash selected"],
	[*],[AC_MSG_ERROR(["invalid --with-verrou-det-hash : ", $vg_cv_verrou_det_hash])])

AC_SUBST(vg_cv_verrou_det_hash)

AC_CACHE_CHECK([verrou-denorm-hack], vg_cv_verrou_denorm_hack,
[
AC_ARG_WITH(
	[verrou-denorm-hack],
	[  --with-verrou-denorm-hack=hack_name	hack avtivated to deals with denorm : none,float,double],
	[vg_cv_verrou_denorm_hack=$withval],
	[vg_cv_verrou_denorm_hack=float]
)])

AS_CASE([$vg_cv_verrou_denorm_hack],
	[yes],[vg_cv_verrou_denorm_hack=float],
	[none],[echo "none denorm hack selected"],
	[float],[echo "float denorm hack selected"],
	[double],[echo "double dernorm hack selected"],
	[all],[echo "all denorm hack selected"],
        [*],[AC_MSG_ERROR(["invalid --with-verrou-denorm-hack : ", $vg_cv_verrou_denorm_hack])])



AM_CONDITIONAL([USE_VERROU_DENORM_HACK_FLOAT], test x$vg_cv_verrou_denorm_hack = xfloat -o  x$vg_cv_verrou_denorm_hack = xall,[])
AM_CONDITIONAL([USE_VERROU_DENORM_HACK_DOUBLE], test x$vg_cv_verrou_denorm_hack = xdouble  -o  x$vg_cv_verrou_denorm_hack = xall,[])


AC_SUBST(vg_cv_verrou_denorm_hack)


# avx512
vg_test_no_avx512f_flag=-mno-avx512f
AC_LANG_PUSH(C++)
CXXFLAGS="$safe_CXXFLAGS $vg_test_no_avx512f_flag"
AC_MSG_CHECKING([for -mno-avx512f compil option])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include <stdlib.h>
      ]], [[return EXIT_SUCCESS;]])],
    [
      CXXFLAGS="$safe_CXXFLAGS"
      AC_MSG_RESULT([yes])
    ],[
    CXXFLAGS="$safe_CXXFLAGS"
     vg_test_no_avx512f_flag=
      AC_MSG_RESULT([no])
  ])
AC_LANG_POP(C++)
AC_SUBST(vg_test_no_avx512f_flag)

# mfpmath=sse
vg_test_mfpmath_sse_flag="-mfpmath=sse"
AC_LANG_PUSH(C++)
CXXFLAGS="$safe_CXXFLAGS $vg_test_mfpmath_sse_flag"
AC_MSG_CHECKING([for -mfpmath=sse compil option])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include <stdlib.h>
      ]], [[return EXIT_SUCCESS;]])],
    [
      CXXFLAGS="$safe_CXXFLAGS"
      AC_MSG_RESULT([yes])
    ],[
    CXXFLAGS="$safe_CXXFLAGS"
     vg_test_mfpmath_sse_flag=
      AC_MSG_RESULT([no])
  ])
AC_LANG_POP(C++)
AC_SUBST(vg_test_mfpmath_sse_flag)

# mfma
vg_test_mfma_flag=-mfma
AC_LANG_PUSH(C++)
CXXFLAGS="$safe_CXXFLAGS $vg_test_mfma_flag"
AC_MSG_CHECKING([for -mfma compil option])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include <stdlib.h>
      ]], [[return EXIT_SUCCESS;]])],
    [
      CXXFLAGS="$safe_CXXFLAGS"
      AC_MSG_RESULT([yes])
    ],[
    CXXFLAGS="$safe_CXXFLAGS"
     vg_test_mfma_flag=
      AC_MSG_RESULT([no])
  ])
AC_LANG_POP(C++)
AC_SUBST(vg_test_mfma_flag)

AC_SUBST(ac_have_as_avx)


AC_CONFIG_FILES([verrou/env.sh])
AC_CONFIG_FILES([verrou/unitTest/flag.mk])
