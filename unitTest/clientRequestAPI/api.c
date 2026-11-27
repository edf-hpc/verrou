/* Client request API (verrou.h): the header is used without any definition
 * from the program, and the values returned by the requests are printed.
 * Natively, every request returns its default value (0). */
#include <stdio.h>
#include "verrou.h"

int main(void)
{
    printf("VERROU_API_VERSION %d\n", VERROU_API_VERSION);
    printf("api_version %u\n", VERROU_GET_API_VERSION);
    printf("rounding %d\n", VERROU_GET_ROUNDING);
    printf("is_instrumented_double %d\n", (int)VERROU_IS_INSTRUMENTED_DOUBLE);
    printf("seed %llu\n", (unsigned long long)VERROU_GET_SEED);
    return 0;
}
