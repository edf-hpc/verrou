# Performance of the delta-debug algorithms

Tools to study the delta-debug algorithms of `pyTools/DD_stoch.py` without running the real code.
They are not installed : they use the `pyTools` directory of this source tree.

## Workflow

1. Build a metamodel from the cache of a real delta-debug (any `verrou_dd_*` variant) :

       verrou_dd_metamodel [--ratio=R --nruns=M] dd.sym        # -> dd.sym/metamodel.json

   The metamodel is anonymized (the deltas are only indexes in the ordered search space) : it can be
   shared to build a collection of industrial cases.

2. Simulate a delta-debug on the metamodel with the options of the algorithm :

       ddPerf/verrou_dd_fake --cache=clean --nruns=5 --num-threads=4 --rddmin=d metamodel.json

   The algorithms of `DD_stoch` are used unchanged : only the samples are simulated (failure drawn with
   the failure probability of the metamodel, duration given by its time model, or 1 per sample without
   time model) and the thread pool is replaced by a virtual time executor. The simulated wall time thus
   takes into account `--num-threads=`, the early exits and the scheduling of the samples.
   `--trace-file=` writes a trace with the simulated time (`verrou_dd_trace_convert` works on it).
   `--fake-seed=` changes the simulated samples, `--summary=FILE` writes the results in json :
   wall and cpu time, number of samples, ddmin found with their status (`clause` : a clause of the
   metamodel, `contains a clause`, `background` : no clause, the ddmin is due to the diffuse error).

3. Compare option sets on several metamodels and seeds :

       ddPerf/campaign.py --seeds=20 meta1.json meta2.json -- "--nruns=5 --num-threads=4" "--nruns=5 --num-threads=4 --rddmin=s"

   The table gives the means of the wall time, cpu time, number of samples, number of ddmin, fraction of
   the clauses found (recall) and number of background ddmin (noise).

## Limits

- The samples of a configuration are independent (the correlation between configurations computed with
  the same `--seed=` by verrou is not modeled).
- The clauses of a metamodel are the ddmin found by the original delta-debug (minus the ones absorbed in
  the background) : a clause missed by the original delta-debug is not in the metamodel.
