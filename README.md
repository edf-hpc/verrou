# Verrou [![Build Status](https://travis-ci.org/edf-hpc/verrou.svg?branch=master)](https://travis-ci.org/edf-hpc/verrou)

Verrou helps you look for floating-point round-off errors in programs. It
implements a stochastic floating-point arithmetic based on random rounding: all
floating-point operations are perturbed by randomly switching rounding
modes. This can be seen as an asynchronous variant of the CESTAC method, or a
subset of Monte Carlo Arithmetic, performing only output randomization through
random rounding.

**NB:** This is the *development* version of Verrou, currently based on the
latest stable release of Valgrind, v3.13.0. For other versions, please consult
the list of [releases](https://github.com/edf-hpc/verrou/releases).


## Installation

### Get the sources

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/tags/VALGRIND_3_13_0 valgrind

Add verrou's sources to it:

    cd valgrind
    git clone --branch=master --single-branch https://github.com/edf-hpc/verrou.git verrou

    patch -p0 <verrou/valgrind.diff


### Configure and build

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

You can also add the `--enable-verrou-fma=yes` flag to the configuration above
if your system supports FMA instructions. Depending on system it may be required to set CFLAGS :
   ./configure --enable-only64bit --enable-verrou-fma=yes --prefix=PREFIX CFLAGS="-march=native -mfma"



Build and install:

    make
    make install


### Load the environment

In order to actually use Verrou, you must load the correct environment. This can
be done using:

    source PREFIX/env.sh


### Test (optional)

#### General tests

You can test the whole platform:

    make check
    perl tests/vg_regtest --all
    
or only verrou:

    make -C tests check
    make -C verrou check
    perl tests/vg_regtest verrou
    
    
#### Specific tests

These tests are more closely related to the arithmetic part in Verrou:

    make -C verrou/unitTest


## Documentation

The documentation for verrou is available as a
[chapter in valgrind's manual](//edf-hpc.github.com/verrou/vr-manual.html).

<p>&nbsp;</p>

You can also re-build it:

    make -C docs html-docs man-pages

and browse it locally:

    iceweasel docs/html/vr-manual.html


Beware, this requires lots of tools which are not necessarily tested for in
`configure`, including (but not necessarily limited to):

  - xsltproc
  - docbook-xsl

