#include "integrate.hxx"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <functional>
#include <sstream>

typedef float RealType;

/**  
Tests the convergence of the integral calculation with the number of rectangles.

The test case is the calculation of the integral of cos between 0 and pi/2,
whose exact value is 1.

The same calculation is performed for an increasing (~geometric) sequence of
number of rectangles. Each calculation result is displayed as output.

@param step factor between two numbers of rectangles tested
 */

void testConvergence (const RealType & step) {
  std::cout << std::scientific << std::setprecision(17);
  const unsigned int maxN= 10000;//0;
  for (unsigned int n = 1 ; n <= maxN ;
       n = std::max (std::min(maxN, (unsigned int)(step*n)), n+1)) {

    RealType res = integrate ((RealType(*)(RealType)) std::cos,
			      (RealType)0, (RealType)M_PI_2, n);

    RealType err = std::abs(1 - res);

    // 3 columns output: Nrectangles Result Error
    std::cout << std::setw(10) << n << " " << res << " " << err << std::endl;
  }
}



/** Tools: from string convertion 
    @param str  string input
    @param TO   type of data to read
 */
template <typename TO>
TO strTo (const std::string & str) {
  std::istringstream iss(str);
  TO x;
  iss >> x;
  return x;
}


/* 
   Main function: the step factor of the number of rectangles
   tested is read as the first argument on the command line.
   The default value is 10 if no argument is supplied.
 */
int main (int argc, char **argv) {
  RealType step = 10;
  if (argc > 1)
    step = strTo<RealType> (argv[1]);

  testConvergence (step);
  return 0;
}
