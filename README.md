# Verrou

Verrou helps you look for floating-point round-off errors in programs. It
implements a stochastic floating-point arithmetic based on random rounding: all
floating-point operations are perturbed by randomly switching rounding
modes. This can be seen as an asynchronous variant of the CESTAC method, or a
subset of Monte Carlo Arithmetic, performing only output randomization through
random rounding.

**NB:** This is released version 0.9 of Verrou. It is based on Valgrind-3.10.1,
which supports the following system configurations:

- `gcc`, versions 3.x to 5.x
- `clang`, versions 2.9 to 4.x
- `glibc`, versions 2.2 to 2.20


## Installation

The installation procedure described here has been tested only on Linux
platforms. It may also work for MacOS, but this has never been tested.


### Get the sources

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/tags/VALGRIND_3_10_1 valgrind


Add verrou's sources to it:

    cd valgrind
    git clone https://github.com/edf-hpc/verrou.git verrou

    patch -p1 <verrou/valgrind.diff


### Configure and build

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

Build and install:

    make
    make install


### Test (optional)

Build the tests:

    make check

... and run them:

    # run all tests:
    perl tests/vg_regtest --all

    # or test only verrou:
    perl tests/vg_regtest verrou


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


## Contributing

Please let us know if you used Verrou (successfully or not), and even more if it
helped you improve the quality of your code.

Also, if you make improvements to this code or have suggestions, please do not
hesitate to fork the repository or submit bug reports on
[github](https://github.com/edf-hpc/verrou). The repository's URL is:

    https://github.com/edf-hpc/verrou.git


### Contributors

- [F. Févotte](https://github.com/ffevotte)
- [B. Lathuilière](https://github.com/lathuili)



## License

Copyright (C) 2014-2017, EDF.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.

The GNU General Public License is contained in the file COPYING.
