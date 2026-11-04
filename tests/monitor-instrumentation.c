/* The argument hard|soft of the monitor command instrumentation is optional */
#include <stdio.h>
#include "../../include/valgrind.h"

static void monitor (char* command) {
  fprintf (stderr, "%s\n", command);
  VALGRIND_MONITOR_COMMAND (command); /* prints the instrumentation status */
}

int main (int argc, char** argv) {
  monitor ("instrumentation off");
  monitor ("instrumentation on");
  monitor ("instrumentation off soft");
  monitor ("instrumentation on soft");
  monitor ("instrumentation on unknown");
  return 0;
}
