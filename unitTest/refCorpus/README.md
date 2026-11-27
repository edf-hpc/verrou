# R1 reference corpus

Task R1 of the "verrou 3.0" plan (phase R): freeze the behaviour of
verrou-Valgrind before the rewrite, so that the new version can be compared to
it bit for bit in every reproducible mode.

## Usage

```bash
make                                   # builds the programs (build/)
make check VALGRIND=valgrind           # compares to ref/ (a few seconds)
make gen   VALGRIND=valgrind           # regenerates ref/ (reference verrou)
make bench VALGRIND=valgrind           # performance bench (about 3 min)
./corpus.py list                       # list of cases
./corpus.py check --only 'ops\.upward' --keep /tmp/res   # one case, results kept
```

In the source tree, without installation:
`VALGRIND=$PWD/../../../vg-in-place` (the `verrou_dd_*` tools are taken from
`../../pyTools` when they are not in the `PATH`).

`VALGRIND` can be any command accepting `--tool=verrou` followed by verrou
options: this is how the new version (`verrou` launcher, R2) will be compared
to the corpus.

## Content

| Program | What it exercises |
|---|---|
| `src/ops.c` | All the instrumented operations: add, sub, mul, div, sqrt, FMA, conversions; float and double scalars, SSE (2 doubles, 4 floats) and AVX (4 doubles, 8 floats) packed. Special values (±0, denormals, `DBL_MAX`, infinities, exact cases) and fixed pseudo-random values, including cancellations. 18,816 results |
| `src/algos.c` | Common algorithms: naive and compensated sums, vectorized dot product, axpy, Horner, Muller recurrence, Newton, Gauss on a Hilbert matrix, 2D stencil, libm calls. 1 or 4 threads |

Each result is printed exactly (`%a`).

| Cases (77) | Modes, options | Reference outputs |
|---|---|---|
| `ops.MODE`, `algos1.MODE`, `algos4.MODE` | 21 reproducible modes (all programs) + 4 seeded modes (single thread) | numerical output, counters |
| `algos1.artefacts` | nearest | `--gen-exclude`, `--gen-source`, coverage (`--trace`, reduced to symbol/file/line) |
| `algos1.upward.exclude_kernels` | `--exclude` of the kernel | native kernel results |
| `algos1.upward.source_subset` | `--source` (2 lines) | only these lines perturbed |
| `ops.iomatch_startstop.{upward,random_det}` | IOMatch: `stop` then `start` when some lines are printed, `display_counter`, `reset_counter` | IOMatch log, output, counters |
| `algos1.verrou_dd_line` | `verrou_dd_line`, `random_det`, seed 1 | ddmin found |

**Modes.** Reproducible whatever the scheduling (checked with 4 threads):
`nearest upward downward toward_zero away_zero farthest float native ftz daz
dazftz`, the hash-based variants `random_det random_comdet random_scomdet
nearness_det nearness_comdet nearness_scomdet prandom_det prandom_comdet`, and
`sr_monotonic sr_smonotonic`. With a seed, in a single thread only (the draws
follow the order of the operations): `random nearness prandom random_instr`;
this part is specific to verrou-Valgrind, as another engine may draw in
another order. Seed: `--vr-seed=42`.

## Reference format (`ref/`)

- `stdout`: complete output when it has at most 200 lines; otherwise a digest
  by group (first word of the line: number of lines and SHA-256), which
  points to the faulty operation (`dadd`, `avx_fmadd`...). The complete outputs
  can be regenerated with the reference verrou-Valgrind.
- `counters`: operation counters table of the log, without pid or path.
- `gen_exclude`, `gen_source`, `source_list`: paths reduced to the file name, sorted lines.
- `cover`: `symbol  file  line  executions`.
- `iomatch_log`, `dd_result`: IOMatch log, ddmin (source lines).
- `META`: valgrind and compiler versions, seed, **SHA-256 of the binaries**.
  The references are valid for these binaries: `check` warns when the
  binaries have changed (other compiler, other options). Source line numbers
  appear in the references: changing `src/algos.c` requires regenerating them.
- `bench_baseline.csv`: one bench measurement (Haswell i5-4460), not compared
  automatically. `make gen` erases `ref/`: redo it with
  `./bench.sh ref/bench_baseline.csv`.

## Not covered yet

Instrumented libm (`--libm=instrumented`), `back_cover` traces, other tools
(`verrou_dd_sym`, `_task`, `_stdout`, `_back`, `post_verrou_dd`,
`verrou_plot_stat`), `verrou_control`, client requests, NaN/Inf and denormal
detection (errors), `mcaquad`, `vprec` and `checkcancellation` backends, x87,
AVX-512 (no AVX-512 machine), MPI.
