/* One NaN, one Inf and one denormal result, to check the suppressions */
volatile double zero=0., one=1., tiny=1e-160;

int main (int argc, char** argv) {
  double nan=zero/zero;
  double inf=one/zero;
  double denorm=tiny*tiny;
  return (nan==nan) || (inf==1.) || (denorm==1.);
}
