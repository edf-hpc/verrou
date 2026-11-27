# Performance of the delta-debug algorithms

Tools to study the delta-debug algorithms of `pyTools/DD_stoch.py` without running the real code.
They are not installed : they use the `pyTools` directory of this source tree.

## Workflow

1. Build a metamodel from the cache of a real delta-debug (any `verrou_dd_*` variant) :

       verrou_dd_metamodel [--ratio=R --nruns=M] dd.sym        # -> dd.sym/metamodel.json
       verrou_dd_metamodel [--ratio=R --nruns=M] dd.sym dd.sym.seed2 ...   # several delta-debug of the same code merged

   The metamodel is anonymized (the deltas are only indexes in the ordered search space) : it can be
   shared to build a collection of industrial cases.

2. Simulate a delta-debug on the metamodel with the options of the algorithm :

       ddPerf/verrou_dd_fake --cache=clean --nruns=5 --num-threads=4 --rddmin=d metamodel.json

   The algorithms of `DD_stoch` are used unchanged : only the samples are simulated (failure drawn with
   the failure probability of the metamodel, duration given by its time model, or 1 per sample without
   time model) and the thread pool is replaced by a virtual time executor. The simulated wall time thus
   takes into account `--num-threads=`, the early exits and the scheduling of the samples.
   `--trace-file=` writes a trace with the simulated time (`verrou_dd_trace_convert` works on it).
   By default the diffuse background of the metamodel is used : none for a metamodel fitted by
   `verrou_dd_metamodel` without `--background` (its default). `--no-background` ignores the background of
   a metamodel fitted with `--background` (only the clauses fail).
   `--fake-seed=` fixes the simulated samples (by default a random seed is drawn and printed), `--summary=FILE` writes the results in json :
   wall and cpu time, number of samples, ddmin found with their status (`clause` : a clause of the
   metamodel, `contains a clause`, `graded clause part` : some deltas of a graded clause (an accumulation of
   small errors is split in several ddmin, the graded clause is counted as found),
   `background` : no clause, the ddmin is due to the diffuse error).
   The summary also gives the rddmin-cmp configuration (complement of the ddmin found) : its samples, its failure
   probability in the metamodel and the clauses it still contains (a final check passed by chance).

3. Compare option sets on one or several metamodels and seeds, histograms of the cost (option sets superposed,
   at most 3 by figure) :

       ddPerf/histograms.py --seeds=100 meta1.json meta2.json -- "--nruns=5 --num-threads=4" "--nruns=5 --num-threads=4 --rddmin=s"

   The number of samples and the simulated wall time are measured at the end of the delta-debug, when the
   first ddmin is found, when half of the clauses of the metamodel (rounded up) are found and when all the clauses
   are found (the seeds which do not reach a step are counted in the figure title). The quality of the result is
   given by the percentage of the clauses of the metamodel found (also weighted by the failure probability of each
   clause), the failure probability of the complement of the ddmin found (probability that the final check passes
   although the code still fails), the parallel efficiency, the number of deltas of the ddmin outside the clauses
   (they should be in the complement rddmin-cmp : with `--no-background` they are useless for the user) and the
   number of non minimal ddmin (a clause and other deltas). The figure is written in `histograms/<metamodel>.png`, the raw
   values in `histograms/<metamodel>.values.json`, and the quantiles with a compact histogram are printed.
   A compact table ends the output : one line by metamodel and option set with the number of seeds and of
   stopped simulations, the means (on all the seeds, the stopped simulations with their values at the stop) of
   the wall time, cpu time, number of samples, number of ddmin and of background ddmin (noise), of the clauses
   found (also weighted) and of the failure probability of the complement of the ddmin found (resPFail).
   The instants are also in the summary of `verrou_dd_fake` (`foundTime` of each ddmin, `progress`).
   `--max-samples=M` stops each simulation after M samples, so that an algorithm out of control does not block
   the histograms : a stopped simulation is not counted in the complete step.
   The summary of each simulation is kept in a cache (`histograms/work/`, by metamodel content, option set and
   seed) : adding an option set or increasing `--seeds=` only computes the missing simulations. After a change
   of `DD_stoch` or `verrou_dd_fake`, `--recompute` computes again all the simulations.

`verrou_dd_fake --max-samples=M` stops the simulation after M samples : the summary (`stopped`) gives the ddmin
found before the stop.

## Limits

- The samples of a configuration are independent (the correlation between configurations computed with
  the same `--seed=` by verrou is not modeled) : `--seed=` is not an option of `verrou_dd_fake`.
- The clauses of a metamodel come from the ddmin found by the original delta-debug (minus the ones absorbed
  in the background with `verrou_dd_metamodel --background`, merged in graded clauses when they are the parts of an accumulation of small errors) :
  a cause missed by the original delta-debug is only in the metamodel if the enrichment finds it (localized
  residual, alternative clause : substitute in the residual of a part of a ddmin) or if it is a ddmin of
  another cache given to `verrou_dd_metamodel`.
