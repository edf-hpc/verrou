# Installation

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/trunk valgrind


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

    ./autogen.sh
    ./configure --enable-only64bit


Build and install

    make
    make install


# Usage

    valgrind --tool=verrou [OPTIONS] INSTRUMENTED_PROGRAM


## Options

- `--rounding-mode=[random|average]`:
  - `random`: Change rounding mode at each FP instruction. The rounding mode is
    chosen randomly among the 4 standard IEEE-754 modes, with equal
    probabilities.
  - `average`: Change rounding mode at each FP instruction. The rounding mode is
    chosen randomly among `UPWARD` and `DOWNWARD`, with probabilities such that
    the expectation of the rounded value is the real operation result.

- `instr-atstart=[yes|no]`: determine whether instrumentation is turned on at
  program start. It can later be switched using "client requests".


## Client requests

    #include "valgrind/verrou.h"


    // Start instrumenting
    VERROU_START_INSTRUMENTATION;


    // Stop instrumenting
    VERROU_STOP_INSTRUMENTATION;
