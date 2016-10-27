# Installation

## Get the sources

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/tags/VALGRIND_3_10_1 valgrind


Add verrou's sources to it:

    cd valgrind
    git clone https://github.com/edf-hpc/verrou.git verrou

    patch -p1 <verrou/valgrind.diff


## Configure and build

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

Build and install:

    make
    make install


## Test (optional)

Build the tests:

    make check

... and run them:

    # run all tests:
    perl tests/vg_regtest --all

    # or test only verrou:
    perl tests/vg_regtest verrou


# Documentation

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

