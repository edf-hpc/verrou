#!/usr/bin/env python3
"""R1 reference corpus: behaviour of verrou to freeze before the rewrite.

  corpus.py gen   [--valgrind CMD] [--jobs N] [--only REGEX]   : writes ref/
  corpus.py check [--valgrind CMD] [--jobs N] [--only REGEX] [--keep DIR]
  corpus.py list                                               : list of cases

Each case runs a program of build/ under verrou with a rounding mode and
options, then normalizes what it produces:
  stdout    : numerical output (%a); complete if it is short, otherwise a
              digest by group of lines (first word: number of lines and
              SHA-256), which points to the faulty group in case of mismatch;
  counters  : operation counters table of the verrou log;
  and depending on the case: generated exclusions and source lines
  (--gen-exclude, --gen-source), coverage by source line (--trace), IOMatch
  log, ddmin found by verrou_dd_line.
The normalization removes what depends on the execution (pid, paths,
addresses).

The references depend on the binaries: ref/META records the SHA-256 of their
code (without debug information), the valgrind and compiler versions. check
reports different binaries.
"""

import argparse
import concurrent.futures
import hashlib
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(HERE, "build")
REF = os.path.join(HERE, "ref")
SEED = "42"

# Modes reproducible whatever the thread scheduling: deterministic modes,
# hash-based variants (_det, _comdet, _scomdet), sr_*.
DET_MODES = ["nearest", "upward", "downward", "toward_zero", "away_zero", "farthest", "float",
             "native", "ftz", "daz", "dazftz",
             "random_det", "random_comdet", "random_scomdet",
             "nearness_det", "nearness_comdet", "nearness_scomdet",
             "prandom_det", "prandom_comdet",
             "sr_monotonic", "sr_smonotonic"]
# Modes with a pseudo-random generator: reproducible with --vr-seed in a
# single thread only (the draws follow the order of the operations).
# Reference specific to verrou-Valgrind: another engine may draw in another order.
SEEDED_MODES = ["random", "nearness", "prandom", "random_instr"]

# vprec backend configurations (deterministic, also with threads)
VPREC = [("vprec_default", []),
         ("vprec_bf16", ["--vprec-preset=bfloat16"]),
         ("vprec_fp16", ["--vprec-preset=binary16"]),
         ("vprec_tf32", ["--vprec-preset=tf32"]),
         ("vprec_p30_full", ["--vprec-mode=full", "--vprec-precision-binary64=30", "--vprec-range-binary64=8"]),
         ("vprec_p30_ib", ["--vprec-mode=ib", "--vprec-precision-binary64=30"]),
         ("vprec_ieee", ["--vprec-mode=ieee"]),
         ("vprec_p40_dazftz", ["--vprec-precision-binary64=40", "--vprec-daz=yes", "--vprec-ftz=yes"])]

FULL_STDOUT_MAX = 200


class Case:
    def __init__(self, name, argv, opts, kind="run", env=None, extra=None):
        self.name = name
        self.argv = argv
        self.opts = opts
        self.kind = kind
        self.env = env or {}
        self.extra = extra or {}


def cases():
    res = []
    progs = [("ops", ["ops"], True), ("algos1", ["algos", "1"], True), ("algos4", ["algos", "4"], False)]
    for pname, argv, single in progs:
        modes = DET_MODES + (SEEDED_MODES if single else [])
        for m in modes:
            res.append(Case("%s.%s" % (pname, m), argv, ["--rounding-mode=" + m, "--vr-seed=" + SEED]))
    # vprec backend
    for vname, vopts in VPREC:
        for pname, argv in [("ops", ["ops"]), ("algos1", ["algos", "1"])]:
            res.append(Case("%s.%s" % (pname, vname), argv, ["--backend=vprec"] + vopts))
    res.append(Case("algos4.vprec_bf16", ["algos", "4"], ["--backend=vprec", "--vprec-preset=bfloat16"]))
    # client requests (verrou.h): values returned, results, log messages
    for m, extra in [("random_det", []), ("nearest", []), ("upward", []), ("upward.instr_atstart_no", ["--instr-atstart=no"])]:
        res.append(Case("clreq.%s" % m, ["clreq"], ["--rounding-mode=" + m.split(".")[0], "--vr-seed=" + SEED] + extra,
                        kind="log"))
    # NaN and Inf detection: valgrind errors with their location
    for name, opts in [("nearest", ["--rounding-mode=nearest"]), ("upward", ["--rounding-mode=upward"]),
                       ("random_det", ["--rounding-mode=random_det"]),
                       ("nocheck", ["--rounding-mode=nearest", "--check-nan=no", "--check-inf=no"])]:
        res.append(Case("naninf.%s" % name, ["naninf"], opts + ["--vr-seed=" + SEED], kind="log"))
    # denormals: checkdenorm backend (errors, counters, generated source lines, daz/ftz)
    cd = ["--backend=checkdenorm", "--vr-seed=" + SEED]
    res.append(Case("denorm.check_count_gen", ["denorm"], cd + ["--check-denorm=yes", "--count-denorm=yes"], kind="denorm"))
    res.append(Case("denorm.count", ["denorm"], cd + ["--count-denorm=yes"], kind="log"))
    for m in ["ftz", "daz", "dazftz"]:
        res.append(Case("denorm.%s" % m, ["denorm"], cd + ["--rounding-mode=" + m, "--count-denorm=yes"], kind="log"))
    # artefacts: generated exclusions and source lines, coverage
    res.append(Case("algos1.artefacts", ["algos", "1"], ["--rounding-mode=nearest"], kind="artefacts"))
    # exclusion of the computation kernel: the kernel results become native again
    res.append(Case("algos1.upward.exclude_kernels", ["algos", "1"],
                    ["--rounding-mode=upward"], kind="exclude", extra={"symbols": ["kernels"]}))
    # instrumentation restricted to the lines of the sum and of Muller
    res.append(Case("algos1.upward.source_subset", ["algos", "1"],
                    ["--rounding-mode=upward"], kind="source", extra={"patterns": ["sd +=", "u2 = 111.0"]}))
    # delta-debug by lines: random_det and fixed seed, hence deterministic;
    # failure if Horner (res 6) or Muller (res 9) deviate from the nearest computation
    res.append(Case("algos1.verrou_dd_line", ["algos", "1"], [], kind="dd"))
    # IOMatch: instrumentation stopped then restarted when some lines are printed
    for m in ["upward", "random_det"]:
        res.append(Case("ops.iomatch_startstop.%s" % m, ["ops"],
                        ["--rounding-mode=" + m, "--vr-seed=" + SEED,
                         "--IOmatch-clr=" + os.path.join(HERE, "iomatch_startstop.script")],
                        kind="iomatch", env={"CORPUS_LINEBUF": "1"}))
    return res


# --------------------------------------------------------------------------
# Normalization

PID_PREFIX = re.compile(r"^==\d+== ?")


def digest_stdout(text):
    lines = text.splitlines()
    if len(lines) <= FULL_STDOUT_MAX:
        return "full %d\n" % len(lines) + "".join(l + "\n" for l in lines)
    groups = []
    data = {}
    for l in lines:
        g = l.split(" ", 1)[0]
        if g not in data:
            data[g] = [0, hashlib.sha256()]
            groups.append(g)
        data[g][0] += 1
        data[g][1].update((l + "\n").encode())
    out = "digest %d\n" % len(lines)
    for g in groups:
        out += "%-14s %6d %s\n" % (g, data[g][0], data[g][1].hexdigest()[:16])
    return out


def counters(stderr):
    """Operation counters table (end of the log), without pid."""
    lines = [PID_PREFIX.sub("", l) for l in stderr.splitlines() if l.startswith("==")]
    out = []
    keep = False
    for l in lines:
        if "Operation" in l and "Instruction count" in l:
            keep = True
        if keep:
            if l.startswith("For lists of detected") or l.startswith("ERROR SUMMARY"):
                break
            if "`/" in l:  # messages with a path (files written by verrou)
                continue
            out.append(l.rstrip())
    while out and out[-1] == "":
        out.pop()
    for l in lines:
        if l.startswith("Backend") and "rounding mode" in l:
            out.insert(0, l.strip())
            break
    return "\n".join(out) + "\n"


FRAME = re.compile(r"^(\s+(?:at|by)) 0x[0-9A-Fa-f]+: ")
BACKQUOTED_PATH = re.compile(r"`(/[^']*)'")


def log(stderr):
    """verrou log after the header: errors with their backtrace (without
    address), messages, counters tables, error summary. Without pid; paths
    reduced to the file name."""
    out = []
    started = False
    in_config = False
    for raw in stderr.splitlines():
        m = re.match(r"^(==|--)\d+(==|--) ?", raw)
        if m is None:
            continue
        l = raw[m.end():]
        if not started:
            started = l.startswith("Command:")
            continue
        if l.startswith("Instrumented operations") or l.startswith("Instrumented vectorized") \
           or l.startswith("Instrumented type"):
            in_config = True
            continue
        if in_config and l.startswith("\t"):
            continue
        in_config = False
        if re.match(r"^Backend \S+ : ", l) or l.startswith("For lists of detected"):
            continue
        l = FRAME.sub(r"\1 ", l)
        # seed of a deterministic section: derived from the pid
        l = re.sub(r"^(Entering deterministic section )\d+:", r"\1<pid-dependent>:", l)
        l = BACKQUOTED_PATH.sub(lambda mm: "`" + os.path.basename(mm.group(1)) + "'", l)
        l = re.sub(r"(EXCLUDE DETECTED: |\t)/\S+/", r"\1", l)
        out.append(l.rstrip())
    while out and out[0] == "":
        out.pop(0)
    return "\n".join(out) + "\n"


def norm_paths(text):
    """Paths replaced by their file name, sorted lines."""
    res = []
    for l in text.splitlines():
        res.append("\t".join(os.path.basename(f) if "/" in f else f for f in l.split("\t")))
    return "\n".join(sorted(res)) + "\n"


def cover_by_line(rep):
    """--trace coverage reduced to (symbol, file, line) -> executions."""
    info = {}
    cov = {}
    for f in os.listdir(rep):
        path = os.path.join(rep, f)
        if f.startswith("trace_bb_info.log"):
            for l in open(path):
                if "|" not in l:
                    continue
                key, rest = l.split("|", 1)
                fields = [x.strip() for x in rest.split(":")]
                if len(fields) >= 4:
                    info.setdefault(key, set()).add((fields[1], fields[2], fields[3]))
        elif f.startswith("trace_bb_cov.log"):
            for l in open(path):
                if "|" in l and ":" in l:
                    key, rest = l.strip().split("|", 1)
                    cov[key] = cov.get(key, 0) + int(rest.split(":")[1])
    agg = {}
    for key, n in cov.items():
        for loc in info.get(key, {("?", "?", "?")}):
            agg[loc] = agg.get(loc, 0) + n
    return "".join("%s\t%s\t%s\t%d\n" % (s, f, l, n) for (s, f, l), n in sorted(agg.items()))


DD_RUN = """#!/bin/bash
# verrou_dd run script: nearest for the reference (set by verrou_dd),
# random_det for the samples (seed VERROU_SEED set by verrou_dd)
export VERROU_ROUNDING_MODE=${VERROU_ROUNDING_MODE:-random_det}
exec %s --tool=verrou %s 1 > "$1/out" 2> "$1/err"
"""

DD_CMP = """#!/usr/bin/env python3
import sys
def res(rep):
    return {l.split()[3]: float.fromhex(l.split()[4]) for l in open(rep + "/out")}
ref, run = res(sys.argv[1]), res(sys.argv[2])
for k in ["6", "9"]:
    if abs(run[k] - ref[k]) > 1e-6 * abs(ref[k]):
        sys.exit(1)
sys.exit(0)
"""


def pytools_dir():
    """verrou_dd_*: installed ones (PATH) or the ones of the source tree"""
    if shutil.which("verrou_dd_line") is not None:
        return None
    return os.path.abspath(os.path.join(HERE, "..", "..", "pyTools"))


def dd_result(rep):
    """ddmin found (sorted source lines)"""
    out = []
    names = sorted(os.listdir(rep))
    for n in names:
        if re.match(r"ddmin\d+$", n):
            inc = os.path.join(rep, n, "dd.line.include")
            lines = norm_paths(open(inc).read()).strip().replace("\n", " ; ")
            out.append("ddmin " + lines)
    out.sort()
    return "\n".join(out) + "\n"


def iomatch_log(rep):
    out = []
    for f in sorted(os.listdir(rep)):
        if ".log-" in f or f.startswith("IOMatch.log"):
            for l in open(os.path.join(rep, f)):
                out.append(l.rstrip())
    return "\n".join(out) + "\n"


# --------------------------------------------------------------------------
# Execution

def run_vg(valgrind, opts, argv, env_extra, cwd):
    cmd = shlex.split(valgrind) + ["--tool=verrou"] + opts + [os.path.join(BUILD, argv[0])] + argv[1:]
    env = dict(os.environ)
    env.update(env_extra)
    p = subprocess.run(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       universal_newlines=True, timeout=1200)
    if p.returncode != 0:
        raise RuntimeError("%s: return code %d\n%s" % (" ".join(cmd), p.returncode, p.stderr[-2000:]))
    return p.stdout, p.stderr


def run_case(case, valgrind):
    """Returns {file name: normalized content}"""
    tmp = tempfile.mkdtemp(prefix="corpus.")
    try:
        files = {}
        opts = list(case.opts)
        if case.kind == "artefacts":
            excl, src = os.path.join(tmp, "excl"), os.path.join(tmp, "src")
            out, err = run_vg(valgrind, opts + ["--gen-exclude=" + excl, "--gen-source=" + src], case.argv, case.env, tmp)
            files["gen_exclude"] = norm_paths(open(excl).read())
            files["gen_source"] = norm_paths(open(src).read())
            trep = os.path.join(tmp, "trace")
            os.mkdir(trep)
            run_vg(valgrind, opts + ["--trace=" + excl, "--output-trace-rep=" + trep], case.argv, case.env, tmp)
            files["cover"] = cover_by_line(trep)
        elif case.kind == "exclude":
            excl = os.path.join(tmp, "excl")
            with open(excl, "w") as f:
                for s in case.extra["symbols"]:
                    f.write("%s\t%s\n" % (s, os.path.join(BUILD, case.argv[0])))
            out, err = run_vg(valgrind, opts + ["--exclude=" + excl], case.argv, case.env, tmp)
        elif case.kind == "source":
            gsrc = os.path.join(tmp, "gsrc")
            run_vg(valgrind, ["--rounding-mode=nearest", "--gen-source=" + gsrc], case.argv, case.env, tmp)
            srcfile = os.path.join(HERE, "src", case.argv[0] + ".c")
            text = open(srcfile).read().splitlines()
            wanted = [i + 1 for i, l in enumerate(text) if any(p in l for p in case.extra["patterns"])]
            sel = [l for l in open(gsrc) if int(l.split("\t")[1]) in wanted]
            if not sel:
                raise RuntimeError("%s: no source line selected" % case.name)
            src = os.path.join(tmp, "src")
            open(src, "w").write("".join(sel))
            files["source_list"] = norm_paths("".join(sel))
            out, err = run_vg(valgrind, opts + ["--source=" + src], case.argv, case.env, tmp)
        elif case.kind == "dd":
            run = os.path.join(tmp, "run.sh")
            open(run, "w").write(DD_RUN % (valgrind, os.path.join(BUILD, case.argv[0])))
            cmp = os.path.join(tmp, "cmp.py")
            open(cmp, "w").write(DD_CMP)
            os.chmod(run, 0o755)
            os.chmod(cmp, 0o755)
            env = dict(os.environ)
            pt = pytools_dir()
            if pt is not None:
                env["PATH"] = pt + os.pathsep + env["PATH"]
                env["PYTHONPATH"] = pt + os.pathsep + env.get("PYTHONPATH", "")
            p = subprocess.run(["verrou_dd_line", "--cache=clean", "--nruns=3", "--num-threads=2", "--seed=1",
                                run, cmp], cwd=tmp, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               universal_newlines=True, timeout=3600)
            if p.returncode != 0:
                raise RuntimeError("verrou_dd_line: return code %d\n%s" % (p.returncode, p.stdout[-2000:]))
            files["dd_result"] = dd_result(os.path.join(tmp, "dd.line"))
            return files
        elif case.kind == "denorm":
            cdi, cdo = os.path.join(tmp, "cdi"), os.path.join(tmp, "cdo")
            out, err = run_vg(valgrind, opts + ["--cdi-gen-file=" + cdi, "--cdo-gen-file=" + cdo], case.argv,
                              case.env, tmp)
            files["cdi_gen"] = norm_paths(open(cdi).read())
            files["cdo_gen"] = norm_paths(open(cdo).read())
            files["stdout"] = digest_stdout(out)
            files["log"] = log(err)
            return files
        elif case.kind == "iomatch":
            rep = os.path.join(tmp, "iom")
            os.mkdir(rep)
            out, err = run_vg(valgrind, opts + ["--output-IOmatch-rep=" + rep], case.argv, case.env, tmp)
            files["iomatch_log"] = iomatch_log(rep)
        else:  # "run" and "log"
            out, err = run_vg(valgrind, opts, case.argv, case.env, tmp)
        if case.kind == "log":
            files["stdout"] = digest_stdout(out)
            files["log"] = log(err)
        elif case.kind != "artefacts":
            files["stdout"] = digest_stdout(out)
            files["counters"] = counters(err)
        else:
            files["counters"] = counters(err)
        return files
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def code_hash(path):
    """SHA-256 of a binary without debug information nor build-id: -g records
    the build directory and the include paths, which differ between checkouts
    while the code is the same. Raw SHA-256 if objcopy is not available."""
    fd, tmp = tempfile.mkstemp(prefix="corpus.bin.")
    os.close(fd)
    try:
        r = subprocess.run(["objcopy", "--strip-all", "--remove-section=.note.gnu.build-id", path, tmp],
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        data = open(tmp if r.returncode == 0 else path, "rb").read()
        return ("" if r.returncode == 0 else "raw ") + hashlib.sha256(data).hexdigest()
    except OSError:
        return "raw " + hashlib.sha256(open(path, "rb").read()).hexdigest()
    finally:
        os.unlink(tmp)


def meta(valgrind):
    def out(cmd):
        try:
            return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  universal_newlines=True).stdout.strip().splitlines()[0]
        except Exception:
            return "?"
    lines = ["valgrind: " + out(shlex.split(valgrind) + ["--version"]),
             "gcc: " + out(["gcc", "--version"]),
             "seed: " + SEED]
    for b in sorted(os.listdir(BUILD)):
        lines.append("binary %s: %s" % (b, code_hash(os.path.join(BUILD, b))))
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("action", choices=["gen", "check", "list"])
    ap.add_argument("--valgrind", default=os.environ.get("VALGRIND", "valgrind"))
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--only", default=None, help="regular expression on the case names")
    ap.add_argument("--keep", default=None, help="check: writes the obtained results in this directory")
    a = ap.parse_args()
    todo = [c for c in cases() if a.only is None or re.search(a.only, c.name)]
    if a.action == "list":
        for c in todo:
            print(c.name)
        return 0
    if a.action == "check" and os.path.isfile(os.path.join(REF, "META")):
        ref_bins = [l for l in open(os.path.join(REF, "META")) if l.startswith("binary")]
        cur_bins = [l + "\n" for l in meta(a.valgrind).splitlines() if l.startswith("binary")]
        if ref_bins != cur_bins:
            print("WARNING: binaries differ from the reference ones (ref/META): mismatches may be unrelated "
                  "to verrou (regenerate the reference with the reference verrou-Valgrind)")
    nfail = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
        futures = {ex.submit(run_case, c, a.valgrind): c for c in todo}
        for fu in concurrent.futures.as_completed(futures):
            c = futures[fu]
            try:
                files = fu.result()
            except Exception as e:
                nfail += 1
                print("ERROR  %s\n%s" % (c.name, e))
                continue
            refdir = os.path.join(REF, c.name)
            if a.action == "gen":
                os.makedirs(refdir, exist_ok=True)
                for k, v in files.items():
                    open(os.path.join(refdir, k), "w").write(v)
                print("gen    %s" % c.name)
                continue
            diffs = []
            for k, v in sorted(files.items()):
                rp = os.path.join(refdir, k)
                if not os.path.isfile(rp):
                    diffs.append("%s (missing from the reference)" % k)
                elif open(rp).read() != v:
                    rl, vl = open(rp).read().splitlines(), v.splitlines()
                    first = next((i for i in range(min(len(rl), len(vl))) if rl[i] != vl[i]), min(len(rl), len(vl)))
                    diffs.append("%s line %d: expected '%s', got '%s'" % (
                        k, first + 1, rl[first] if first < len(rl) else "", vl[first] if first < len(vl) else ""))
            if a.keep:
                os.makedirs(os.path.join(a.keep, c.name), exist_ok=True)
                for k, v in files.items():
                    open(os.path.join(a.keep, c.name, k), "w").write(v)
            if diffs:
                nfail += 1
                print("DIFF   %s\n       %s" % (c.name, "\n       ".join(diffs)))
            else:
                print("ok     %s" % c.name)
    if a.action == "gen" and not a.only:
        open(os.path.join(REF, "META"), "w").write(meta(a.valgrind))
    print("%d cases, %d failed" % (len(todo), nfail))
    return 1 if nfail else 0


if __name__ == "__main__":
    sys.exit(main())
