
/** calculating the integral of a function between two bounds using the rectangles method.
    @param f   the function
    @param x0  left bound
    @param x1  right bound
    @param n   number of rectangles
 */
template <typename T, class REALTYPE>
REALTYPE integrate (const T& f, REALTYPE a, REALTYPE b, unsigned int n) {
  // Integration step
  const REALTYPE dx = (b - a) / n;
  // Naive integration Loop
  REALTYPE sum = 0.;
#ifdef FLOATLOOP
  for (REALTYPE x = a + (REALTYPE)0.5 * dx ;
       x < b ;
       x += dx)
    {
#elif defined(INTEGERLOOP)
      for(unsigned int i =0; i < n; i++)
    {
      REALTYPE x= ((REALTYPE)i + (REALTYPE)0.5) *dx ;
#else
#error "FLOATLOOP or INTEGER LOOP should be defined"
#endif
    sum += dx * f(x);
  }
  return sum;
}
