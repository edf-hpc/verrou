
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
  for (REALTYPE x = a + (REALTYPE)0.5 * dx ;
       x < b ;
       x += dx) {
    sum += dx * f(x);
  }
  return sum;
}
