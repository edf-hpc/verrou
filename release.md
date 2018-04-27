# Notes about the release process

## Update the valgrind patch

```
perl -pni -e 's/verrou-dev/verrou-X.Y.Z/' valgrind.diff
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


## Commit in a release branch

1. Prepare the commit

    ```
    git checkout -b release
    git commit -m "Release vX.Y.Z
    git push origin release
    ```

2. Wait for Travis tests to run

3. Tag and delete the `release` branch

    ```
    git tag vX.Y.Z
    git push origin vX.Y.Z
    git branch -d release
    git push origin :release
    ```


## Merge the released version back in the master branch


1. Prepare a merge in the `master` branch

    ```
    git checkout master
    git merge --no-ff --no-commit release
    ```

2. Revert the change in `README.md` and `valgrind.diff`

3. Add an `[UNRELEASED]` section in `Changelog.md`

4. Commit the merge

    ```
    git commit -m "Release vX.Y.Z"
    git push origin master
    ```

## Add a release message in github

    
