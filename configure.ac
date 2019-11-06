AM_PATH_PYTHON([3])

AC_CACHE_CHECK([verrou fma], vg_cv_verrou_fma,
  [AC_ARG_ENABLE(verrou-fma,
    [  --enable-verrou-fma          enables verrou to use fma],
    [vg_cv_verrou_fma=$enableval],
    [vg_cv_verrou_fma=no])])

if test "$vg_cv_verrou_fma" = yes; then
  # Check for fmaintrin.h
  AC_LANG_PUSH(C++)
  CXXFLAGS="$safe_CXXFLAGS -mfma"
  AC_MSG_CHECKING([for fmaintrin.h ])
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
      AM_CONDITIONAL([HAVE_FMA_INTRIN], test x$vg_cv_verrou_fma = xyes,[])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([A compiler with fmaintrin.h is required for --enable-verrou-fma ])
  ])
  AC_LANG_POP(C++)
else
  AM_CONDITIONAL([HAVE_FMA_INTRIN], test x$vg_cv_verrou_fma = xyes,[])
fi

AC_SUBST(vg_cv_verrou_fma)
AC_CONFIG_FILES([verrou/env.sh])
