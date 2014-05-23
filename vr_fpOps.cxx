#include <cstdlib>
#include <ctime>
#include <cmath>
#include "vr_fpOps.h"

#include "pub_tool_vki.h"

extern "C" {
#include <stdio.h>
#include "pub_tool_libcprint.h"
#include "pub_tool_libcfile.h"
}

#include "vr_fpRepr.hxx"

// * Global variables & parameters

vr_RoundingMode ROUNDINGMODE;
const int CHECK_IP = 0;
const int CHECK_C  = 0;

int vr_outFile;


// * Operation implementation

inline char const*const roundingModeName (vr_RoundingMode mode) {
  switch (mode) {
  case VR_NEAREST:
    return "NEAREST";
  case VR_UPWARD:
    return "UPWARD";
  case VR_DOWNWARD:
    return "DOWNWARD";
  case VR_ZERO:
    return "TOWARD_ZERO";
  case VR_RANDOM:
    return "RANDOM";
  case VR_AVERAGE:
    return "AVERAGE";
  }

  return "undefined";
}

template <typename REAL>
inline vr_RoundingMode getMode (vr_RoundingMode mode, const REAL & err, const REAL & ulp) {
  switch(mode) {
  case VR_RANDOM:
    return (vr_RoundingMode)(rand()%VR_RANDOM);
  case VR_AVERAGE: {
    int s = err>=0 ? 1 : -1;

    if (rand() * ulp < RAND_MAX * s * err) {
      // Probability: abs(err)/ulp
      if (s>0) {
        return VR_UPWARD;
      } else {
        return VR_DOWNWARD;
      }
    } else {
      return VR_NEAREST;
    }
  }
  default:
    return mode;
  }
}

// ** Addition

template <typename REAL>
void checkInsufficientPrecision (const REAL & a, const REAL & b) {
  if (CHECK_IP == 0)
    return;

  const int ea = exponentField (a);
  const int eb = exponentField (b);

  const int emax = ea>eb ? ea : eb;
  const int emin = ea<eb ? ea : eb;

  const int n = significantBits(a);
  const int dp = 1+emax-emin-n;

  char s[256];
  if (dp>0) {
    const int l = VG_(sprintf)(s, "IP %d\n", dp);
    VG_(write)(vr_outFile, s, l);
  }
}

template <typename REAL>
void checkCancellation (const REAL & a, const REAL & b, const REAL & r) {
  if (CHECK_C == 0)
    return;

  const int ea = exponentField (a);
  const int eb = exponentField (b);
  const int er = exponentField (r);

  const int emax = ea>eb ? ea : eb;
  const int cancelled = emax - er;

  char s[256];
  if (cancelled >= significantBits(a)) {
    const int l = VG_(sprintf)(s, "C  %d\n", cancelled);
    VG_(write)(vr_outFile, s, l);
  }
}

template <typename REAL, vr_RoundingMode ROUND>
class Sum {
public:
  struct ValErr {
    REAL value;
    REAL error;
  };

  static REAL apply (const REAL & a, const REAL & b) {
    if (a == 0)
      return b;
    if (b == 0)
      return a;

    ValErr ve = (std::abs(a) < std::abs(b)) ?
      priest_ (b, a):
      priest_ (a, b);

    const REAL u = ulp(ve.value);
    const vr_RoundingMode mode = getMode (ROUND, ve.error, u);

    if (ve.error > 0
        && (mode == VR_UPWARD
            || (mode == VR_ZERO && ve.value < 0)))
      ve.value += u;

    if (ve.error < 0
        && (mode == VR_DOWNWARD
            || (mode == VR_ZERO && ve.value > 0)))
      ve.value -= u;

    checkInsufficientPrecision (a, b);
    checkCancellation (a, b, ve.value);

    return ve.value;
  }

private:
  // Priest's algorithm
  //
  // Priest, D. M.: 1992, "On Properties of Floating Point Arithmetics: Numerical
  // Stability and the Cost of Accurate Computations". Ph.D. thesis, Mathematics
  // Department, University of California, Berkeley, CA, USA.
  //
  // ftp://ftp.icsi.berkeley.edu/pub/theory/priest-thesis.ps.Z
  static ValErr priest_ (const REAL & a, const REAL & b) {
    REAL c = a + b;
    const REAL e = c - a;
    const REAL g = c - e;
    const REAL h = g - a;
    const REAL f = b - h;
    REAL d = f - e;

    if (d + e != f) {
      c = a;
      d = b;
    }

    ValErr s;
    s.value = c;
    s.error = d;
    return s;
  }
};


// * C interface

void vr_fpOpsInit (vr_RoundingMode mode) {
  ROUNDINGMODE = mode;

  if (ROUNDINGMODE >= VR_RANDOM) {
    srand (time (NULL));
  }

  VG_(umsg)("Simulating %s rounding mode\n", roundingModeName (ROUNDINGMODE));

  vr_outFile = VG_(fd_open)("vr.log",
                            VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
                            VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
}

void vr_fpOpsFini (void) {
  VG_(close)(vr_outFile);
}

double vr_AddDouble (double a, double b) {
  switch (ROUNDINGMODE) {
  case VR_NEAREST:
    return Sum<double, VR_NEAREST>::apply (a, b);
  case VR_UPWARD:
    return Sum<double, VR_UPWARD>::apply (a, b);
  case VR_DOWNWARD:
    return Sum<double, VR_DOWNWARD>::apply (a, b);
  case VR_ZERO:
    return Sum<double, VR_ZERO>::apply (a, b);
  case VR_RANDOM:
    return Sum<double, VR_RANDOM>::apply (a, b);
  case VR_AVERAGE:
    return Sum<double, VR_AVERAGE>::apply (a, b);
  }
  return 0;
}

float vr_AddFloat (float a, float b) {
  switch (ROUNDINGMODE) {
  case VR_NEAREST:
    return Sum<float, VR_NEAREST>::apply (a, b);
  case VR_UPWARD:
    return Sum<float, VR_UPWARD>::apply (a, b);
  case VR_DOWNWARD:
    return Sum<float, VR_DOWNWARD>::apply (a, b);
  case VR_ZERO:
    return Sum<float, VR_ZERO>::apply (a, b);
  case VR_RANDOM:
    return Sum<float, VR_RANDOM>::apply (a, b);
  case VR_AVERAGE:
    return Sum<float, VR_AVERAGE>::apply (a, b);
  }
  return 0;
}
