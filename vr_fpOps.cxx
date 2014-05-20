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



// * Global variables & parameters

vr_RoundingMode ROUNDINGMODE;
const int CHECK_IP = 0;
const int CHECK_C  = 0;

int vr_outFile;

// * Real types storage

// ** Internal functions

// IEEE754-like binary floating point number representation:
//
// Real:     corresponding C type (float/double)
// BitField: type of the corresponding bit field
// SIGN:     number of sign bits
// EXP:      number of exponent bits
// MANT:     number of mantissa (aka fraction) bits
template <typename Real, typename BitField, int SIGN, int EXP, int MANT>
class FPRepr {

public:
  typedef Real RealType;

  // Integer value of the exponent field of the given real
  //
  // Warning: this value is shifted. The real exponent of x is:
  //    exponentField(x) - exponentShift()
  //
  // x: floating point value
  static inline int exponentField (const Real & x) {
    const BitField *xx = (BitField*)(&x);
    return bitrange<MANT, EXP> (xx);
  }

  // Smallest floating point increment for a given value.
  //
  // x: floating point value around which to compute the ulp
  static Real ulp (const Real & x) {
    const int exponent = exponentField(x);

    Real ret = 0;
    BitField & ulp = *((BitField*)&ret);

    const int exponentULP = exponent-MANT;
    ulp += ((BitField)exponentULP) << MANT;

#ifdef DEBUG
    std::cerr << "ulp(" << x << ") = " << ret << std::endl;
#endif

    return ret;
  }

  static inline void pp (const Real & x) {
    const BitField *xx = (BitField*)(&x);
    const int sign = bitrange<MANT+EXP, SIGN> (xx);
    const BitField mantissa = bitrange<0, MANT> (xx) + ((BitField)1<<MANT);
    const int exponent = exponentField(x)-exponentShift();

    char const * format;
    switch (sizeof(BitField)) {
    case 4:
      format = "%s%d * 2**%d\n";
      break;
    case 8:
      format = "%s%lu * 2**%d\n";
      break;
    default:
      format = "error";
    }
    VG_(umsg)(format,
              (sign==0?" ":"-"),
              mantissa,
              exponent);
  }

  static inline int significantBits () {
    return MANT;
  }

private:
  static inline int exponentShift () {
    return (1 << (EXP-1)) - 1 + MANT;
  }

  // Return a range in a bit field.
  //
  // BitField: type of the bit field
  // begin:    index of the first interesting bit
  // size:     number of desired bits
  // x:        pointer to the bit field
  template <int BEGIN, int SIZE>
  static inline BitField bitrange (BitField const*const x) {
    BitField ret = *x;

    const int leftShift = 8*sizeof(BitField)-BEGIN-SIZE;
    if (leftShift > 0)
      ret = ret << leftShift;

    const int rightShift = BEGIN + leftShift;
    if (rightShift > 0)
      ret = ret >> rightShift;

    return ret;
  }


};


// ** Interface for simple & double precision FP numbers

template <typename Real> struct FPType;

template <> struct FPType<float> {
  typedef FPRepr<float,  __uint32_t, 1,  8, 23>  Repr;
};

template <> struct FPType<double> {
  typedef FPRepr<double, __uint64_t, 1, 11, 52>  Repr;
};

// Smallest floating point increments for IEE754 binary formats
template <typename Real> Real ulp (const Real & x) {
  return FPType<Real>::Repr::ulp (x);
}

// Pretty-print representation
template <typename Real> void ppReal (const Real & x) {
  FPType<Real>::Repr::pp (x);
}

// Exponent field
template <typename Real> int exponentField (const Real & x) {
  return FPType<Real>::Repr::exponentField (x);
}

// Number of significant bits
template <typename Real> int significantBits (const Real & x) {
  return FPType<Real>::Repr::significantBits();
}


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

  // double d = 42;
  // ppReal (d);

  // float f = 42;
  // ppReal (f);
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
