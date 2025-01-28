#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

void printNaN(double n) {
  std::uint64_t hex;
  std::memcpy(&hex, &n, sizeof n);
  std::cout << n
            << " (" << std::hex << hex << ")"
            << std::endl;
}

int main()
{ double x=cos(42.); if(x!=x) printf ("FAILURE\n");// line to force libm use
  double d = 42.;
  std::cout << "d           = " << d << std::endl;

  double infP = std::numeric_limits<double>::infinity();
  std::cout << "infP        = " << infP << std::endl;

  double infN = -std::numeric_limits<double>::infinity();
  std::cout << "infN        = " << infN << std::endl;
  std::cout << std::endl;

  std::cout << "d / 0       = " << (d / 0.) << std::endl;
  std::cout << "-d / 0      = " << (-d / 0.) << std::endl;
  std::cout << "infP + d    = " << (infP + d) << std::endl;
  std::cout << "infN + d    = " << (infN + d) << std::endl;
  std::cout << std::endl;


  std::cout << "0 / 0       = "; printNaN (0./0.);
  std::cout << "infP + infN = "; printNaN (infP + infN);
  std::cout << "infN + infP = "; printNaN (infN + infP);
  std::cout << "infP - infP = "; printNaN (infP - infP);
  std::cout << "infN - infN = "; printNaN (infN - infN);
  std::cout << std::endl;

  double nan1 = std::nan("0xdead");
  std::cout << "nan1        = "; printNaN (nan1);

  double nan2 = std::nan("0xbabe");
  std::cout << "nan2        = "; printNaN (nan2);
  std::cout << std::endl;

  std::cout << "nan1 + d    = "; printNaN (nan1 + d);
  std::cout << "d + nan1    = "; printNaN (d + nan1);
  std::cout << "nan1 + nan2 = "; printNaN (nan1 + nan2);
  std::cout << "nan2 + nan1 = "; printNaN (nan2 + nan1);
}
