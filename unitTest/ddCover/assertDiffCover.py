#!/usr/bin/env python3

import filecmp
import sys
import re
from pathlib import Path
from sysTools import openGz

NUM_RUN = 2

REP_INTEGER = "dd.line.integer.post"
REP = "dd.line.post"

PID_PATTERN = re.compile(r"\d+")

IGNORED_PATTERNS = [
    re.compile(r"backCoverInfo-\d+"),
    re.compile(r"backCoverInfo-\d+\.gz"),
    re.compile(r"backAddrInfo-\d+\.gz"),
    re.compile(r"backAddrInfo-\d+"),
    re.compile(r"bbAddrInfo-\d+"),
    re.compile(r"bbAddrInfo-\d+\.gz"),
    re.compile(r"pidMap"),
]

def normalize_filename(name):
    """
    Remplace les séquences numériques par <PID>.
    À adapter selon le format réel des fichiers.
    """
    return PID_PATTERN.sub("<PID>", name)

def ignore_filename(path):
    name=path.name
    for creg in IGNORED_PATTERNS:
        if creg.match(name):
            return True
    return False

def cmpCompressedFile(path1, path2):
    tabLines1= openGz(path1,"r").readlines()
    tabLines2= openGz(path2,"r").readlines()

    nb1=len(tabLines1)
    nb2=len(tabLines2)
    if nb1 != nb2:
        print("Wrong number of lines: ", nb1, nb2)
        return False
    else:
        for i in range(nb1):
            line1=tabLines1[i]
            line2=tabLines2[i]
            if line1!=line2:
                print("lines differ index:",i)
                print("line1", line1)
                print("line2", line2)
                return False
        return True


def compute_normalized_dic(directory):
    files = {}
    for path in directory.rglob("*"):
        if ignore_filename(path):
            continue
        if path.is_file():
            rel = path.relative_to(directory)
            normalized = Path(
                *[normalize_filename(part) for part in rel.parts]
            )
            files[str(normalized)] = path
    return files

def directories_are_equal(dir1, dir2):
    files1= compute_normalized_dic(Path(dir1))
    files2= compute_normalized_dic(Path(dir2))

    if set(files1.keys()) != set(files2.keys()):
        print("files1:", files1)
        print("files2:", files2)
        return False

    for key in files1:
        if not cmpCompressedFile(files1[key], files2[key]):
            print("files1[key]:", files1[key])
            print("files2[key]:", files2[key])
            return False
    return True

print("integer loop")

for i in range(NUM_RUN + 1):
    for subrep in ("FullPerturbation", "rddmin-cmp", "ddmin0"):
        dir1 = f"{REP_INTEGER}/{subrep}-trace/nearness_scomdet/dd.run{i}/cover"
        dir2 = f"{REP_INTEGER}/NoPerturbation-trace/default/dd.run0/cover"

        print(f"diff {dir1} {dir2}")

        if not directories_are_equal(dir1, dir2):
            sys.exit(42)

print("float loop")

for i in range(NUM_RUN + 1):
    dir1 = f"{REP}/FullPerturbation-trace/nearness_scomdet/dd.run{i}/cover"
    dir2 = f"{REP}/NoPerturbation-trace/default/dd.run0/cover"

    print(f"diff {dir1} {dir2} diff expected")

    # On attend une différence
    if directories_are_equal(dir1, dir2):
        sys.exit(42)

    # On ne vérifie pas ddmin0 et ddmin1
    for subrep in ("rddmin-cmp",):
        dir1 = f"{REP}/{subrep}-trace/nearness_scomdet/dd.run{i}/cover"
        dir2 = f"{REP}/NoPerturbation-trace/default/dd.run0/cover"

        print(f"diff {dir1} {dir2}")

        if not directories_are_equal(dir1, dir2):
            sys.exit(42)

sys.exit(0)
