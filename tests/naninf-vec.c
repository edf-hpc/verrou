/* NaN/Inf detection on 128-bit vector operations (2 x double and 4 x float).
   GCC vector extensions are used to stay portable between x86_64 (SSE2)
   and arm64 (NEON). Special values are produced in a single lane to check
   that every lane is tested. Results are printed through their bit pattern
   (no floating-point operation, and no dependency on the NaN sign which
   differs between architectures). */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef double v2d __attribute__ ((vector_size (16)));
typedef float  v4f __attribute__ ((vector_size (16)));

/* globals to prevent constant folding */
v2d d_one  = {1., 1.};
v2d d_zero = {0., 0.};
v2d d_big  = {1e308, 1.};
v2d d_lane1Zero = {1., 0.};

v4f f_one  = {1.f, 1.f, 1.f, 1.f};
v4f f_big  = {1.f, 1.f, 3e38f, 1.f};
v4f f_lane3Zero = {1.f, 1.f, 1.f, 0.f};

static const char* classDouble (double x) {
  uint64_t bits;
  memcpy (&bits, &x, sizeof bits);
  if ((bits & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL) return "fin";
  if (bits & 0x000fffffffffffffULL) return "nan";
  return (bits >> 63) ? "-inf" : "+inf";
}

static const char* classFloat (float x) {
  uint32_t bits;
  memcpy (&bits, &x, sizeof bits);
  if ((bits & 0x7f800000U) != 0x7f800000U) return "fin";
  if (bits & 0x007fffffU) return "nan";
  return (bits >> 31) ? "-inf" : "+inf";
}

static void printDouble (const char* msg, v2d v) {
  printf ("%-26s: %s %s\n", msg, classDouble (v[0]), classDouble (v[1]));
}

static void printFloat (const char* msg, v4f v) {
  printf ("%-26s: %s %s %s %s\n", msg,
          classFloat (v[0]), classFloat (v[1]), classFloat (v[2]), classFloat (v[3]));
}

int main (void) {
  v2d d_inf  = d_one / d_lane1Zero;
  printDouble ("double: one / lane1Zero", d_inf);
  printDouble ("double: big + big", d_big + d_big);
  printDouble ("double: big * big", d_big * d_big);
  v2d d_nan  = d_zero / d_lane1Zero;
  printDouble ("double: zero / lane1Zero", d_nan);
  printDouble ("double: inf - inf", d_inf - d_inf);
  printDouble ("double: one + one", d_one + d_one);

  v4f f_inf  = f_one / f_lane3Zero;
  printFloat ("float: one / lane3Zero", f_inf);
  printFloat ("float: big + big", f_big + f_big);
  printFloat ("float: big * big", f_big * f_big);
  printFloat ("float: inf - inf", f_inf - f_inf);
  printFloat ("float: one + one", f_one + f_one);
  return 0;
}
