/* Test program of the verrou command: prints a floating-point result and its
 * arguments, and exits with the code given as first argument. */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    volatile double x = 0.1;
    double s = 0;
    for (int i = 0; i < 1000; i++)
        s += x * i;
    printf("%a\n", s);
    for (int i = 2; i < argc; i++)
        printf("arg %s\n", argv[i]);
    return argc > 1 ? atoi(argv[1]) : 0;
}
