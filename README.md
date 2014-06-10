# Installation

Fetch valgrind's sources:

```sh
svn co svn://svn.valgrind.org/valgrind/trunk valgrind
```


Add verrou's sources to it:

    cd valgrind
    git clone /netdata/H55056/git/verrou.git verrou

    patch -p0 <<EOF
    Index: Makefile.am
    ===================================================================
    --- Makefile.am (révision 13983)
    +++ Makefile.am (copie de travail)
    @@ -10,7 +10,8 @@
                    lackey \
                    none \
                    helgrind \
    -               drd
    +               drd \
    +               verrou
     
     EXP_TOOLS =    exp-sgcheck \
                    exp-bbv \
    Index: configure.ac
    ===================================================================
    --- configure.ac        (révision 13983)
    +++ configure.ac        (copie de travail)
    @@ -3017,6 +3017,8 @@
        exp-bbv/tests/arm-linux/Makefile
        exp-dhat/Makefile
        exp-dhat/tests/Makefile
    +   verrou/Makefile
    +   verrou/tests/Makefile
        shared/Makefile
     ])
     AC_CONFIG_FILES([coregrind/link_tool_exe_linux],
    EOF


Configure valgrind:

```sh
./autogen.sh
./configure --enable-only64bit
```

Build and install

```sh
make
make install
```


# Usage

```sh
valgrind --tool=verrou [OPTIONS] INSTRUMENTED_PROGRAM
```


## Options

- `--rounding-mode=[random|average]`:
  - `random`: Change rounding mode at each FP instruction. The rounding mode is chosen randomly among the 4 standard IEEE-754 modes, with equal probabilities.
  - `average`: Change rounding mode at each FP instruction. The rounding mode is chosen randomly among `UPWARD` and `DOWNWARD`, with probabilities such that the expectation of the rounded value is the real operation result.

- `--instr-atstart=[yes|no]`: determine whether instrumentation is turned on at program start. It can later be switched using "client requests".


## Client requests

```c
#include "valgrind/verrou.h"
```

### Start/stop instrumenting

```c
// Start instrumenting
VERROU_START_INSTRUMENTATION;
```

```c
// Stop instrumenting
VERROU_STOP_INSTRUMENTATION;
```

When encountering these client requests, verrou starts/stop instrumenting the binary code. When not instrumenting, verrou neither counts floating-point instructions, nor changes the rounding mode.


### Enter/leave deterministic sections

#### Basic usage

```c
// Enter a deterministic section
VERROU_START_DETERMINISTIC(0);
```

When encountering this client request, verrou generates a fixed seed for the pseudo-random number generator used by floating-point simulated operations. This seed deterministically depends on the location of the client request within the source files. This ensures that each time the same client request will be emitted, the same seed will be passed to the pRNG.


```c
// Leave a deterministic section
VERROU_STOP_DETERMINISTIC(0);
```

After this client request, the pRNG is re-seeded with a (pseudo-)random value, so as not to be perturbed by sourrounding deterministic sections.


#### Advanced usage

The argument passed to `VERROU_{START,STOP}_DETERMINISTIC` is a level in the stack trace. If not 0, the seed is calculated using the location of the `LEVEL`th call in the stack trace. This can be useful when the call to `VERROU_{START,STOP}_DETERMINISTIC` is encapsulated in a function call.

For example, suppose the following snippet:

```c
void verrou_startDeterministic() {
  VERROU_START_DETERMINISTIC(1);
}

void verrou_stopDeterministic() {
  VERROU_STOP_DETERMINISTIC(1);
}

void deterministic1 () {
  verrou_startDeterministic();
  /* ... */
  verrou_stopDeterministic();
}

void deterministic2 () {
  verrou_startDeterministic();
  /* ... */
  verrou_stopDeterministic();
}
```

When verrou catches the client request, the stack trace could look like:

    #3 main (main.c:80)
    #2 myFunction (main.c:1337)
    #1 deterministic1 (deterministic.c:42)
    #0 verrou_startDeterministic (deterministic.c:34)

or:

    #3 main (main.c:80)
    #2 myFunction (main.c:1342)
    #1 deterministic2 (deterministic.c:47)
    #0 verrou_startDeterministic (deterministic.c:34)

If `VERROU_START_DETERMINISTIC` is called with a 0 LEVEL argument, the pRNG will be seeded in both cases using the same location:

    verrou_startDeterministic (deterministic.c:34)

although there are semantically two distinct deterministic sections, corresponding to functions `deterministic1` and `deterministic2`. The `verrou_startDeterministic` abstraction introduced an unwanted correlation between `deterministic1` and `deterministic2`.

Using `VERROU_START_DETERMINISTIC(1)` tells verrou to look one level up in the stack trace, and seed the pRNG using the location:

    deterministic1 (deterministic.c:42)

or:

    deterministic2 (deterministic.c:47)



Thus, provided that the function calls don't get inlined, the code snippet above is semantically equivalent to: 

```c
void deterministic1 () {
  VERROU_START_DETERMINISTIC(0);
  /* ... */
  VERROU_STOP_DETERMINISTIC(0);
}

void deterministic2 () {
  VERROU_START_DETERMINISTIC(0);
  /* ... */
  VERROU_STOP_DETERMINISTIC(0);
}
```


#### Caveat

1. This feature works best when debug information is present: this helps verrou correctly determine the client request location in the source code.

2. When passing a non-0 LEVEL argument to `VERROU_{START,STOP}_DETERMINISTIC`, be careful that the stack trace depends on the optimization level (in particular when functions get inlined).
