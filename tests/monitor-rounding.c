/* The monitor command rounding needs a rounding mode */
#include <stdio.h>
#include "../../include/valgrind.h"

static void monitor (char* command) {
  fprintf (stderr, "%s\n", command);
  VALGRIND_MONITOR_COMMAND (command);
}

int main (int argc, char** argv) {
  monitor ("rounding");
  monitor ("rounding unknown");
  monitor ("rounding UPWARD");
  fprintf (stderr, "end\n");
  return 0;
}
