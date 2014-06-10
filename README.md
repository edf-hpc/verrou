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
    Index: docs/xml/manual.xml
    ===================================================================
    --- docs/xml/manual.xml  (révision 13983)
    +++ docs/xml/manual.xml  (copie de travail)
    @@ -44,6 +44,8 @@
           xmlns:xi="http://www.w3.org/2001/XInclude" />      
       <xi:include href="../../lackey/docs/lk-manual.xml" parse="xml"  
           xmlns:xi="http://www.w3.org/2001/XInclude" />
    +  <xi:include href="../../verrou/docs/vr-manual.xml" parse="xml"  
    +      xmlns:xi="http://www.w3.org/2001/XInclude" />
       <xi:include href="../../none/docs/nl-manual.xml" parse="xml"  
           xmlns:xi="http://www.w3.org/2001/XInclude" />
    EOF


Configure valgrind:

```sh
./autogen.sh
./configure --enable-only64bit
```

Build and install:

```sh
make
make install
```


Run tests:

```sh
make check
 #
 # run all tests:
perl tests/vg_regtest --all
 #
 # or test only verrou:
perl tests/vg_regtest verrou
```


Build the documentation and browse it:

```sh
cd docs; make html-docs
iceweasel html/vr-manual.html
```
