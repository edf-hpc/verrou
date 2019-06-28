
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Utilities for easier manipulation of floating-point values.  ---*/
/*---                                                vr_fpRepr.hxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2016
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#pragma once
//#include <string>
//#include <sstream>
#include <math.h>
#include <cfloat>
#include <stdint.h>
#include <limits>
#ifdef VALGRIND_DEBUG_VERROU
extern "C" {
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
}
#endif

//#include "interflop_verrou.h"


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
    const BitField *xx = (const BitField*)(&x);
    return bitrange<MANT, EXP> (xx);
  }

  // Smallest floating point increment for a given value.
  //
  // x: floating point value around which to compute the ulp
  static Real ulp (const Real & x) {
    const int exponent = exponentField(x);

    Real ret = 0;
    BitField & ulp = *((BitField*)&ret);
    int exponentULP = exponent-MANT;

    if (exponentULP < 0) {
      // ULP is a subnormal number:
      //    exp  = 0
      //    mant = 1 in the last place
      exponentULP = 0;
      ulp += 1;
    }

    ulp += ((BitField)exponentULP) << MANT;

    return ret;
  }

  static int sign(const Real& x){
    const BitField *xx = (BitField*)(&x);
    const int sign = bitrange<MANT+EXP, SIGN> (xx);
    return sign;
  }


  static inline void pp (const Real & x) {
    //    std::ostringstream oss;

    const BitField *xx = (BitField*)(&x);
    const int sign = bitrange<MANT+EXP, SIGN> (xx);
    const int expField = exponentField(x);
    BitField mantissa = bitrange<0, MANT> (xx);
    int exponent = expField-exponentShift();

    if (expField == 0) {
      // Subnormal floating-point number
      exponent += 1;
    } else {
      // Normal floating-point number
      mantissa += ((BitField)1<<MANT);
    }

    //    oss << (sign==0?" ":"-") << mantissa << " * 2**" << exponent;
#ifdef VALGRIND_DEBUG_VERROU
    VG_(printf)( (sign==0?" ":"-"));
    VG_(printf)("%lu",mantissa);
    VG_(printf)(" * 2**%d  ", exponent);
#endif
  //    return oss.str();
  }




  static inline int storedBits () {
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
  typedef FPRepr<float, uint32_t, 1,  8, 23>  Repr;
};

template <> struct FPType<double> {
  typedef FPRepr<double, uint64_t, 1, 11, 52>  Repr;
};

// Smallest floating point increments for IEE754 binary formats
template <typename Real> Real ulp (const Real & x) {
  return FPType<Real>::Repr::ulp (x);
}

// Pretty-print representation
/*
template <typename Real> std::string ppReal (const Real & x) {
  return FPType<Real>::Repr::pp (x);
  }*/

// Exponent field
template <typename Real> int exponentField (const Real & x) {
  return FPType<Real>::Repr::exponentField (x);
}

// Number of significant bits
template <typename Real> int storedBits (const Real & x) {
  return FPType<Real>::Repr::storedBits();
}

// sign
template <typename Real> int sign (const Real & x) {
  return FPType<Real>::Repr::sign(x);
}



