#include <cstdlib>
#include <ctime>
#include <cmath>
#include "vr_fpOps.h"

// * Real types storage

// Return a range in a bit field.
//
// BitField: type of the bit field
// begin:    index of the first interesting bit
// size:     number of desired bits
// x:        pointer to the bit field
template <int BEGIN, int SIZE, typename BitField>
inline BitField bitrange (BitField const*const x) {
  BitField ret = *x;

  const int leftShift = 8*sizeof(BitField)-BEGIN-SIZE;
  if (leftShift > 0)
    ret = ret << leftShift;

  const int rightShift = BEGIN + leftShift;
  if (rightShift > 0)
    ret = ret >> rightShift;

  return ret;
}

// Compute the smallest floating point increment for a given value.
//
// Floating point number is supposed to use an IEEE754-like binary format:
// BitField: type of the corresponding bit field
// SIGN:     number of sign bits
// EXP:      number of exponent bits
// MANT:     number of mantissa (aka fraction) bits
//
// x:        floating point value around which to compute the ulp
template <typename BitField, int SIGN, int EXP, int MANT, typename Real>
Real computeUlp (const Real & x) {
  const BitField *xx = (BitField*)(&x);

  const int exponent = bitrange<MANT, EXP> (xx);

#ifdef DEBUG
  const int sign = bitrange<MANT+EXP, SIGN> (xx);
  const BitField mantissa = bitrange<0, MANT> (xx) + ((BitField)1<<MANT);
  const int exponentShift = (1 << (EXP-1)) - 1;

  std::cerr << x << " = ";
  if (sign == 0) { std::cerr << " "; }
  else           { std::cerr << "-"; }
  std::cerr << mantissa
            << " * 2**" << exponent-exponentShift-MANT << std::endl;
#endif

  Real ret = 0;
  BitField & ulp = *((BitField*)&ret);

  const int exponentULP = exponent-MANT;
  ulp += ((BitField)exponentULP) << MANT;

#ifdef DEBUG
  std::cerr << "ulp(" << x << ") = " << ret << std::endl;
#endif

  return ret;
}

// Smallest floating point increments for IEE754 binary formats
template <typename REAL> REAL ulp (const REAL & x);

// Single precision
template <> float ulp (const float & x) {
  return computeUlp<__uint32_t, 1, 8, 23> (x);
}

// Double precision
template <> double ulp (const double & x) {
  return computeUlp<__uint64_t, 1, 11, 52> (x);
}


// * Operation implementation

enum RoundingMode {
  VR_NEAREST,
  VR_UPWARD,
  VR_DOWNWARD,
  VR_ZERO,
  VR_RANDOM // Must be in last position!
};

inline RoundingMode getMode (RoundingMode mode) {
  if (mode == VR_RANDOM) {
    return (RoundingMode)(rand()%VR_RANDOM);
  }
  return mode;
}

// ** Addition

template <typename REAL, RoundingMode ROUND>
class Sum {
public:
  struct ValErr {
    REAL value;
    REAL error;
  };

  static REAL apply (const REAL & a, const REAL & b) {
    ValErr ve = (std::abs(a) < std::abs(b)) ?
      priest_ (b, a):
      priest_ (a, b);

    const RoundingMode mode = getMode (ROUND);

    if (ve.error > 0
        && (mode == VR_UPWARD
            || (mode == VR_ZERO && ve.value < 0)))
      ve.value += ulp(ve.value);

    if (ve.error < 0
        && (mode == VR_DOWNWARD
            || (mode == VR_ZERO && ve.value > 0)))
      ve.value -= ulp(ve.value);

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

const RoundingMode ROUNDINGMODE = VR_RANDOM;

void vr_fpOpsInit (void) {
  if (ROUNDINGMODE == VR_RANDOM) {
    srand (time (NULL));
  }
}

double vr_AddDouble (double a, double b) {
  return Sum<double, ROUNDINGMODE>::apply (a, b);
}

float vr_AddFloat (float a, float b) {
  return Sum<float, ROUNDINGMODE>::apply (a, b);
}
