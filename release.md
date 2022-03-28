# Notes about the release process

```
VERSION=2.3.0
```

## Update the valgrind patch

```
perl -pni -e "s/verrou-dev/verrou-${VERSION}/" valgrind.diff
```

## Update README.md

> **NB:** This is released version X.Y.Z of Verrou, based on Valgrind
> vX.Y.Z. The development version of Verrou can always be found in
> the [`master`](https://github.com/edf-hpc/verrou/) branch. For other versions,
> please consult the list
> of [releases](https://github.com/edf-hpc/verrou/releases).


## Update release notes

- Valgrind version
- Added features
- Other changes

- update the Verrou version + release date

    ```
    perl -pni -e "s/\[UNRELEASED\]/v${VERSION} - $(date +%Y-%m-%d)/" CHANGELOG.md
    ```

## Update CLO

```
(cd .. && ./verrou/docs/update-vr-clo)
git add vr_clo.txt
```

## Commit in a release branch

1. Prepare the commit

    ```
    git checkout -b release
    git add -- CHANGELOG.md README.md valgrind.diff
    git commit -m "Release v${VERSION}
      $(perl -n                        \
        -e '$q=1 if m/^\-\-\-/;'       \
        -e 'print if ($p and not $q);' \
        -e '$p=1 if m/^## /;'          \
        CHANGELOG.md)"
    git push origin release
    ```

2. Wait for Travis tests to run

3. Tag and delete the `release` branch

    ```
    git tag v${VERSION}
    git push origin v${VERSION}
    ```


## Merge the released version back in the master branch


1. Prepare a merge in the `master` branch

    ```
    git checkout master
    git merge --no-ff --no-commit v${VERSION}
    git reset HEAD README.md valgrind.diff
    git checkout -- README.md valgrind.diff
    ```

2. Add an `[UNRELEASED]` section in `CHANGELOG.md`:

    > ## [UNRELEASED]
    > 
    > This version is based on Valgrind-3.17.0.
    > 
    > ### Added
    > 
    > 
    > ### Changed
    > 
    > 
    > ---
    > 

3. Commit the merge

    ```
    git commit -m "Post-release v${VERSION}"
    git push origin master
    git branch -d release
    git push origin :release
    ```

## Add a release in github


- Add a release message extracted from CHANGELOG.md

- Build a tgz archive for the full valgrind+verrou release
    
    ```
    VALGRIND=valgrind-3.15.0
    cd /tmp
    wget https://github.com/edf-hpc/verrou/releases/download/valgrind/${VALGRIND}.tar.bz2
    tar xvpf ${VALGRIND}.tar.bz2
    mv ${VALGRIND} ${VALGRIND}+verrou-${VERSION}
    cd ${VALGRIND}+verrou-${VERSION}
    git clone --branch=v${VERSION} --single-branch https://github.com/edf-hpc/verrou
    rm -rf verrou/.git
    patch -p1 <verrou/valgrind.diff
    cd ..
    tar cvzf ${VALGRIND}_verrou-${VERSION}.tar.gz ${VALGRIND}+verrou-${VERSION}
    ```

- Test the archive

    ```
    cd ${VALGRIND}+verrou-${VERSION}
    ./autogen.sh
    ./configure --enable-only64bit --enable-verrou-fma --prefix=$PWD/install
    make -j4 install
    source install/env.sh && valgrind --version
    make -C tests check && make -C verrou check && perl tests/vg_regtest verrou
    make -C verrou/unitTest
    ```

## Update the documentation

```
./verrou/docs/update-docs
```
