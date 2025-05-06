# Verrou

[![Build Status](https://travis-ci.org/edf-hpc/verrou.svg?branch=master)](https://travis-ci.org/edf-hpc/verrou) 
[![Documentation](https://img.shields.io/badge/docs-latest-blue.svg)](http://edf-hpc.github.io/verrou/vr-manual.html)

Verrou helps you look for floating-point round-off errors in programs. It
implements various forms of arithmetic, including:

- all IEEE-754 standard rounding modes;

- three variants of stochastic floating-point arithmetic based on random rounding:
  all floating-point operations are perturbed by randomly switching rounding
  modes. These can be seen as an asynchronous variant of the CESTAC method, or a
  subset of Monte Carlo Arithmetic, performing only output randomization through
  random rounding;

- an emulation of single-precision rounding, in order to test the effect of
  reduced precision without any need to change the source code.

Verrou also comes with a `verrou_dd` utility, which simplifies the Verrou-based
debugging process by implementing several variants of the Delta-Debugging
algorithm. This allows easily locating which parts of the analyzed source code
are likely to be responsible for Floating-Point-related instabilities.

The documentation for Verrou is available as a dedicated [chapter in the
Valgrind manual](http://edf-hpc.github.io/verrou/vr-manual.html).


## Installation

### Get the sources

The preferred way to get Verrou sources is to download the latest *stable*
version: [v2.5.0](https://github.com/edf-hpc/verrou/releases/latest).
Older versions are available in the [releases](https://github.com/edf-hpc/verrou/releases)
page. After downloading one of the released versions, skip to the "Configure
and build" section below.

<p>&nbsp;</p>

In order to build the *development* version of Verrou, it is necessary to first
download a specific Valgrind version, and patch it. Fetch valgrind's sources:

    git clone --branch=VALGRIND_3_25_0 --single-branch https://sourceware.org/git/valgrind.git valgrind-3.25.0+verrou-dev


Add verrou's sources to it:

    cd valgrind-3.25.0+verrou-dev
    git clone https://github.com/edf-hpc/verrou.git verrou

    cat verrou/valgrind.*diff | patch -p1


### Configure and build

First, install all required dependencies (the names of relevant Debian packages
are put in parentheses as examples):

- C & C++ compilers (`build-essential`),
- autoconf & automake (`automake`),
- Python 3 (`python3`),
- C standard library with debugging symbols (`libc6-dbg`),
- [Optional] Python post-treatment tools (`python3-numpy`, `python3-matplotlib`, `texlive-latex-extra`, `texlive-fonts-recommended`, `dvipng`, `cm-super`).

<p>&nbsp;</p>

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

<p>&nbsp;</p>

Advanced users can use the following configure flags :
- `--enable-verrou-fma=yes|no (default yes)`. This option is useful if your system does not support fma intrinsics.
- `--enable-verrou-sqrt=yes|no (default yes)`. This option is useful if your system does not support sqrt intrinsics.
- `--enable-verrou-check-naninf=yes|no` (default yes). If NaN does not appear in the verified code set this option to 'no' can slightly speed up verrou.
- `--with-det-hash=hash_name` with hash_name in [dietzfelbinger,multiply_shift,double_tabulation,xxhash,mersenne_twister] to select the hash function used for [random|average]_[det|comdet] rounding mode. The default is xxhash. double_tabulation was the previous default(before introduction of xxhash). mersenne_twister is the reference but slow. dietzfelbinger and multiply_shift are faster but are not able to reproduce the reference results.
- `--with-verrou-denorm-hack=[none|float|double|all]` (default float). With denormal number the EFT are no more necessary exact. With the average* rounding modes this problem is always ignored, but the random* rounding, there are the following available options :  with  `none` the problem is ignored. With `float` a hack based on computation in double is applied on float operations ; With `double` an experimental hack is applied for double operations ; With `all` the float and double hacks are applied. float is selected by default.
- `--enable-verrou-xoshiro=[no|yes]` (default yes). If set to yes the tiny mersenne twister prng is replaced (for random, prandom and average) by the xo[ro]shiro prng.
- `--enable-verrou-quad=[yes|no]` (default yes). If set to no the backend mcaquad is disabled. This option is only useful to reduce the dependencies.

<p>&nbsp;</p>

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
[chapter in valgrind's manual](//edf-hpc.github.io/verrou/vr-manual.html).

<p>&nbsp;</p>

You can also re-build it:

    make -C docs html-docs man-pages

and browse it locally:

    firefox docs/html/vr-manual.html


Beware, this requires lots of tools which are not necessarily tested for in
`configure`, including (but not necessarily limited to):

  - xsltproc
  - docbook-xsl


## Bibliography & References

The following papers explain in more details the internals of Verrou, as well as
some of its applications. If you use Verrou for a research work, please consider
citing one of these references:

1. François Févotte and Bruno Lathuilière. Debugging and optimization of HPC
   programs with the Verrou tool. In *International Workshop on Software
   Correctness for HPC Applications (Correctness)*, Denver, CO, USA,
   Nov. 2019. [DOI: 10.1109/Correctness49594.2019.00006](http://dx.doi.org/10.1109/Correctness49594.2019.00006)
1. Hadrien Grasland, François Févotte, Bruno Lathuilière, and David
   Chamont. Floating-point profiling of ACTS using Verrou. *EPJ Web Conf.*, 214, 2019.
   [DOI: 10.1051/epjconf/201921405025](http://dx.doi.org/10.1051/epjconf/201921405025)
1. François Févotte and Bruno Lathuilière. Studying the numerical quality of an
   industrial computing code: A case study on code_aster. In *10th International
   Workshop on Numerical Software Verification (NSV)*, pages 61--80, Heidelberg,
   Germany,
   July 2017. [DOI: 10.1007/978-3-319-63501-9_5](http://dx.doi.org/10.1007/978-3-319-63501-9_5)
1. François Févotte and Bruno Lathuilière. VERROU: a CESTAC evaluation without
   recompilation. In *International Symposium on Scientific Computing, Computer
   Arithmetics and Verified Numerics (SCAN)*, Uppsala, Sweden, September 2016.

(These references are also available in [bibtex format](verrou.bib))
