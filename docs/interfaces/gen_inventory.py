#!/usr/bin/env python3

# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2026 EDF
#   B. Lathuilière <bruno.lathuiliere@edf.fr>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.

# The GNU Lesser General Public License is contained in the file COPYING.

"""Inventory of the user interfaces of verrou, extracted from the sources.

  gen_inventory.py          : writes inventory.md (next to this script)
  gen_inventory.py --check  : fails if inventory.md is not up to date

Extracted: options of the verrou tool (vr_clo.c) and their documentation
(vr_clo.txt), environment variables, client requests (verrou.h), monitor
commands (vr_clreq.c), IOMatch keys and actions (vr_IOMatch_clr.c), options
and environment variables of the python tools, existing deprecations.
The inventory is the reference of the interfaces to keep with a new
implementation: any change of an interface appears as a change of this file.
"""

import ast
import importlib.abc
import importlib.machinery
import importlib.util
import inspect
import io
import os
import re
import sys
import contextlib

sys.dont_write_bytecode = True  # no __pycache__ in the source tree

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(HERE, "inventory.md")


def read(name):
    with open(os.path.join(SRC, name)) as f:
        return f.read()


C_TOKEN = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|/\*.*?\*/|//[^\n]*', re.S)


def read_c(name):
    """C source without its comments (the commented out options are not interfaces)"""
    def repl(m):
        t = m.group(0)
        if t.startswith("/"):
            return re.sub(r"[^\n]", " ", t)
        return t
    return C_TOKEN.sub(repl, read(name))


def md_table(header, rows):
    out = "| " + " | ".join(header) + " |\n|" + "---|" * len(header) + "\n"
    for r in rows:
        out += "| " + " | ".join(str(c).replace("|", "\\|") for c in r) + " |\n"
    return out


# --------------------------------------------------------------------------
# verrou tool

CLO = re.compile(r'VG_(XACT|STR|BOOL|INT|BINT|DBL|USET)_CLOM?\s*\(\s*(?:cloPD\s*,\s*)?arg\s*,\s*"([^"]+)"')


def tool_options():
    """{option: {"values": [...], "kind": ..., "deprecated": [messages]}} in source order"""
    text = read_c("vr_clo.c")
    start = text.index("Bool vr_process_clo")
    text = text[start:text.index("\nvoid vr_print_usage", start)]
    matches = list(CLO.finditer(text))
    res = {}
    for k, m in enumerate(matches):
        kind, name = m.group(1), m.group(2)
        block = text[m.end():matches[k + 1].start() if k + 1 < len(matches) else len(text)]
        value = None
        if kind == "XACT" and "=" in name:
            name, value = name.split("=", 1)
            name += "="
        elif kind in ("STR", "INT", "BINT", "DBL", "USET"):
            name += "="
        elif kind == "BOOL":
            name += "="
            value = "yes|no"
        opt = res.setdefault(name, {"values": [], "kind": kind.lower(), "deprecated": []})
        if value is not None and value not in opt["values"]:
            opt["values"].append(value)
        dep = re.search(r'"([^"]*deprecated[^"]*)"', block)
        if dep:
            opt["deprecated"].append((value or name, dep.group(1).replace("\\n", "").strip()))
    return res


def expand(name):
    """--vr-instr-vec<2,4,8>= and --vr-instr-[flt|dbl]= notations of vr_clo.txt"""
    m = re.search(r"<([^>]*)>|\[([^\]]*)\]", name)
    if m is None:
        return [name]
    alts = (m.group(1) or "").split(",") if m.group(1) is not None else m.group(2).split("|")
    res = []
    for a in alts:
        res += expand(name[:m.start()] + a + name[m.end():])
    return res


def documented_options():
    names = set()
    for line in read("vr_clo.txt").splitlines():
        m = re.match(r"^\s+(--[A-Za-z0-9_<>,|\[\]-]+=?)", line)
        if m:
            names.update(expand(m.group(1)))
    return names


def env_variables():
    return re.findall(r'vr_env_clo_one_option\s*\(\s*"(\w+)"\s*,\s*"(--[\w-]+)"\s*\)', read_c("vr_clo.c"))


def client_requests():
    text = read_c("verrou.h")
    enum = text[text.index("VR_USERREQ__START_INSTRUMENTATION = "):]
    enum = enum[:enum.index("}")]
    names = re.findall(r"(VR_USERREQ__\w+)", enum)
    macros = {}
    for m in re.finditer(r"#define\s+(VERROU_\w+)(\([^)]*\))?\s*\\?\s*\n?((?:.*\\\n)*.*)", text):
        body = m.group(3)
        req = re.search(r"(VR_USERREQ__\w+)", body)
        if req is None:
            continue
        body = body.replace("VERROU_ROUNDING_MODE_TYPE", "int")
        kind = "expression" if "_EXPR" in body else "statement"
        cast = re.search(r"\((\w[\w ]*)\)\s*VALGRIND_DO_CLIENT_REQUEST_EXPR", body)
        macros.setdefault(req.group(1), []).append((m.group(1) + (m.group(2) or ""), kind,
                                                    cast.group(1) if cast else
                                                    ("UWord (no cast)" if kind == "expression" else "")))
    rows = []
    for k, n in enumerate(names):
        for macro, kind, cast in macros.get(n, [("", "", "")]):
            rows.append(("base+%d" % k, n.replace("VR_USERREQ__", ""), macro, kind, cast))
    return rows


def rounding_values():
    """values of enum vr_RoundingMode, returned by VERROU_GET_ROUNDING"""
    text = read_c("interflop_backends/interflop_verrou/interflop_verrou.h")
    body = text[text.index("enum vr_RoundingMode"):]
    body = body[body.index("{") + 1:body.index("}")]
    rows = []
    value = -1
    for item in body.split(","):
        item = item.strip()
        if not item or item == "VR_ENUM_SIZE":
            continue
        if "=" in item:
            item, v = [x.strip() for x in item.split("=")]
            value = int(v)
        else:
            value += 1
        rows.append((value, item))
    return rows


def monitor_commands():
    text = read_c("vr_clreq.c")
    helps = re.findall(r'VG_\(gdb_printf\)\("  (\S[^:]*?)\s*:\s*([^"]*?)\\n"\)', text)
    return [(c.strip(), d.strip()) for c, d in helps]


def iomatch():
    text = read_c("vr_IOMatch_clr.c")
    keys = []
    actions = []
    for name, value in re.findall(r'static const HChar (\w+)Str\[\]\s*=\s*"([^"]*)"', text):
        if name.startswith("vr_") and name.endswith("Key"):
            keys.append(value.strip())
        elif value:
            actions.append(value)
    return keys, actions


LIB_SOURCES = ["Interlibmath/interlibmath.cxx", "libVerrouTask/libverrouTask.cxx",
               "extendStdoutPrecision.cxx", "unbufferPrintf.c", "launcher/verrou"]


def library_env_variables():
    """environment variables read by the libraries preloaded in the program
    and by the launcher"""
    rows = []
    for name in LIB_SOURCES:
        if not os.path.exists(os.path.join(SRC, name)):
            continue
        text = read_c(name) if name.endswith((".c", ".cxx")) else read(name)
        found = re.findall(r'getenv\s*\(\s*"(\w+)"', text) + re.findall(r"\$\{?(VERROU_\w+)", text)
        found += ["VERROU_" + v for v in re.findall(r'getenvTask\s*\(\s*"(\w+)"', text)]
        for var in sorted(set(found)):
            rows.append((var, name))
    return rows


# --------------------------------------------------------------------------
# python tools

def bin_scripts():
    am = read("Makefile.am")
    line = am[am.index("bin_SCRIPTS"):]
    line = line[:line.index("\n")]
    return [s.replace("${PYTHON_REP}", "pyTools") for s in line.split("=", 1)[1].split()]


class _Stub:
    """value of any attribute of a missing optional module"""
    def __getattr__(self, name):
        return _Stub()

    def __call__(self, *args, **kwargs):
        return _Stub()


class _StubModule(type(sys)):
    __path__ = []

    def __getattr__(self, name):
        if name.startswith("__"):
            raise AttributeError(name)
        return _Stub()


class _StubFinder(importlib.abc.MetaPathFinder, importlib.abc.Loader):
    """last finder of sys.meta_path: a module not installed (matplotlib,
    scipy, paraview...) is replaced by a stub, so that the inventory does not
    depend on the optional modules installed"""
    def find_spec(self, name, path, target=None):
        return importlib.util.spec_from_loader(name, self)

    def create_module(self, spec):
        return _StubModule(spec.name)

    def exec_module(self, module):
        pass


def load_script(path):
    sys.path.insert(0, os.path.dirname(path))
    if not any(isinstance(f, _StubFinder) for f in sys.meta_path):
        sys.meta_path.append(_StubFinder())
    loader = importlib.machinery.SourceFileLoader("inv_" + re.sub(r"\W", "_", os.path.basename(path)), path)
    spec = importlib.util.spec_from_loader(loader.name, loader)
    module = importlib.util.module_from_spec(spec)
    with contextlib.redirect_stdout(io.StringIO()):
        loader.exec_module(module)
    return module


def config_keys(cls):
    """config_keys of a gen_config subclass: literal in its __init__ or default value"""
    for klass in cls.__mro__:
        if "__init__" not in vars(klass):
            continue
        src = inspect.getsource(klass.__init__)
        m = re.search(r"config_keys\s*=\s*(\[[^\]]*\])", src) or \
            re.search(r"super\(\)\.__init__\(\s*argv\s*,\s*environ\s*,\s*(\[[^\]]*\])", src)
        if m:
            return ast.literal_eval(m.group(1))
    return []


def python_tools():
    import gen_config
    res = []
    for script in bin_scripts():
        path = os.path.join(SRC, script)
        name = os.path.basename(script)
        try:
            src = open(path).read()
        except (OSError, UnicodeDecodeError):
            continue
        # only the scripts based on gen_config are loaded: the result does
        # not depend on the optional modules installed (paraview...)
        if not re.search(r"^import (gen_config|dd_config|post_config)\b", src, re.M):
            envs = sorted(set(re.findall(r"os\.environ\[\s*[\"'](\w+)[\"']\s*\]", src)))
            env_txt = (" ; environment: " + ", ".join(envs)) if envs else ""
            if "argparse" in src:
                opts = sorted(set(re.findall(r"add_argument\(\s*(?:'-\w',\s*)?'(--[\w-]+)'", src)))
                res.append((name, "argparse" + env_txt, [(o, "") for o in opts]))
            elif "getopt" in src:
                opts = sorted(set(re.findall(r'"(--[\w-]+)"', src)))
                res.append((name, "getopt" + env_txt, [(o, "") for o in opts]))
            else:
                opts = sorted(set(re.findall(r'"(--[\w-]+)"\s+in\s+sys\.argv', src)))
                res.append((name, ("own parsing of sys.argv" if opts else "no option") + env_txt,
                            [(o, "") for o in opts]))
            continue
        try:
            module = load_script(path)
        except BaseException as e:  # tools which need optional modules (paraview, matplotlib...)
            res.append((name, "not loaded (%s)" % type(e).__name__, []))
            continue
        def is_config(v):
            return isinstance(v, type) and issubclass(v, gen_config.gen_config) and v is not gen_config.gen_config
        # configuration class defined in the script, otherwise in an imported *_config module
        classes = [v for v in vars(module).values() if is_config(v) and v.__module__ == module.__name__]
        if not classes:
            for m in vars(module).values():
                if inspect.ismodule(m) and m.__name__.endswith("_config") and m.__name__ != "dd_config":
                    classes += [v for v in vars(m).values() if is_config(v) and v.__module__ == m.__name__]
        if not classes:
            res.append((name, "configuration class not found", []))
            continue
        cls = classes[-1]
        # registerOptions without __init__ : the attributes set by __init__
        # (checkParam functions...) are only stored in the registry
        probe = type("Probe", (cls,), {"__getattr__": lambda self, name: None})
        obj = probe.__new__(probe)
        obj.registryTab = []
        obj.prefix_ = "PREFIX"
        try:
            with contextlib.redirect_stdout(io.StringIO()):
                obj.registerOptions()
        except Exception as e:
            res.append((name, "options not extracted (%s: %s)" % (type(e).__name__, e), []))
            continue
        keys = config_keys(cls)
        rows = []
        for r in obj.registryTab:
            env = ", ".join(k + "_" + r["ENV"] for k in keys) if r["ENV"] else ""
            rows.append((" ".join(r["tabOptionVisible"]), env))
        res.append((name, "gen_config (environment: %s)" % (", ".join(k + "_*" for k in keys) or "none"), rows))
    return res


def python_deprecations():
    rows = []
    for root in ["pyTools"]:
        for f in sorted(os.listdir(os.path.join(SRC, root))):
            p = os.path.join(SRC, root, f)
            if not os.path.isfile(p) or f.endswith((".pyc",)):
                continue
            try:
                text = open(p).read()
            except UnicodeDecodeError:
                continue
            for m in re.finditer(r'print\(\s*"([^"]*deprecated[^"]*)"', text, re.IGNORECASE):
                defs = re.findall(r"^\s*def (\w+)", text[:m.start()], re.M)
                where = root + "/" + f + (" (%s)" % defs[-1] if defs else "")
                row = (where, m.group(1).strip())
                if row not in rows:
                    rows.append(row)
    return rows


# --------------------------------------------------------------------------

def generate():
    sys.path.insert(0, os.path.join(SRC, "pyTools"))
    out = io.StringIO()
    w = out.write
    w("# Inventory of the verrou interfaces\n\n")
    w("Generated by `docs/interfaces/gen_inventory.py` from the sources: do not edit.\n")
    w("Any change of an interface of verrou appears as a change of this file.\n\n")

    opts = tool_options()
    doc = documented_options()
    env = env_variables()
    env_by_opt = {}
    for e, o in env:
        env_by_opt.setdefault(o + "=", []).append(e)
        env_by_opt.setdefault(o, []).append(e)
    w("## Options of the verrou tool\n\n")
    w("Parsed by `vr_process_clo` (`vr_clo.c`); documented in `vr_clo.txt`.\n\n")
    rows = []
    for name, o in opts.items():
        envs = env_by_opt.get(name, [])
        rows.append((name, " ".join(o["values"]) if o["values"] else o["kind"], ", ".join(envs),
                     "yes" if name in doc or name.rstrip("=") in doc else "**no**"))
    w(md_table(["option", "values", "environment variable", "documented"], rows))
    undoc = [n for n in sorted(doc) if n not in opts and n + "=" not in opts and n.rstrip("=") not in opts
             and n.rstrip("=") + "=" not in opts]
    if undoc:
        w("\nDocumented in `vr_clo.txt` but not parsed by `vr_process_clo`: %s\n" % ", ".join("`%s`" % n for n in undoc))
    w("\n## Environment variables of the verrou tool\n\n")
    w("Read by `vr_env_clo` (`vr_clo.c`), equivalent to the option.\n\n")
    w(md_table(["variable", "option"], sorted(env)))

    w("\n## Environment variables of the libraries and of the launcher\n\n")
    w(md_table(["variable", "read by"], library_env_variables()))

    w("\n## Client requests (`verrou.h`)\n\n")
    w("Numbered from `VG_USERREQ_TOOL_BASE('V','R')` in the order of the enum: the numbers are the ABI.\n\n")
    w(md_table(["number", "request", "macro", "kind", "return type"], client_requests()))

    w("\nValues returned by `VERROU_GET_ROUNDING`, `VERROU_GET_LIBM_ROUNDING` and "
      "`VERROU_GET_LIBM_ROUNDING_NO_INST` (enum `vr_RoundingMode` of `interflop_verrou.h`, "
      "part of the ABI):\n\n")
    w(md_table(["value", "rounding mode"], rounding_values()))

    w("\n## Monitor commands (`vr_clreq.c`, used by `verrou_control`)\n\n")
    w(md_table(["command", "description"], monitor_commands()))

    keys, actions = iomatch()
    w("\n## IOMatch script (`vr_IOMatch_clr.c`)\n\n")
    w("Keys: %s\n\n" % ", ".join("`%s`" % k for k in keys))
    w("Values of `apply:` / `post-apply:` / `default:` and of the other keys: %s\n" % ", ".join(
        "`%s`" % a for a in actions))

    w("\n## Python tools\n\n")
    for name, kind, rows in python_tools():
        w("### %s\n\n%s\n\n" % (name, kind))
        if rows:
            w(md_table(["option", "environment variables"], rows) + "\n")

    w("## Existing deprecations\n\n")
    w("`read_environ` of `gen_config.py`: every option of the python tools given by "
      "an environment variable (`VERROU_DD_*`, `PLOT_STAT_*`, `POST_*`...) prints the warning.\n\n")
    rows = []
    for name, o in opts.items():
        for value, msg in o["deprecated"]:
            rows.append(("verrou " + (name + value if value != name else name), msg))
    rows += python_deprecations()
    for name in LIB_SOURCES:
        if name.endswith((".c", ".cxx")) and os.path.exists(os.path.join(SRC, name)):
            for v in sorted(set(re.findall(r'getenvTask\s*\(\s*"(\w+)"', read_c(name)))):
                rows.append(("environment variable %s (%s)" % (v, name), "use VERROU_%s instead" % v))
    w(md_table(["interface", "message"], rows))
    return out.getvalue()


def main():
    text = generate()
    if "--check" in sys.argv:
        with open(OUT) as f:
            if f.read() != text:
                print("inventory.md is not up to date: run docs/interfaces/gen_inventory.py")
                return 1
        print("inventory.md up to date")
        return 0
    with open(OUT, "w") as f:
        f.write(text)
    print("written: " + OUT)
    return 0


if __name__ == "__main__":
    sys.exit(main())
