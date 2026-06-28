# Benchmarks

## Contention Benchmark (`contention`)

Quantifies the global-mutex bottleneck in `FixedThreadPool` at minimal task work,
establishing a baseline to beat in later stages.

### Methodology

- **Pool:** `ts::FixedThreadPool` with 1, 2, 4, 8 worker threads.
- **Tasks:** 10,000 per iteration. Each spins for 0 µs (no-op + atomic increment)
  to expose pure lock contention.
- **Build:** Release (`-O2`), no sanitizers, real-time wall-clock measurement.
- **Tool:** Google Benchmark.

### Results

```
Benchmark                            Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------
BM_CONTENTION/1/0/real_time       1.13 ms         1.08 ms          634 items_per_second=8.84624M/s
BM_CONTENTION/2/0/real_time      0.855 ms        0.718 ms          783 items_per_second=11.6895M/s
BM_CONTENTION/4/0/real_time       1.89 ms         1.61 ms          367 items_per_second=5.2921M/s
BM_CONTENTION/8/0/real_time       4.72 ms         4.00 ms          153 items_per_second=2.11671M/s
```

### Interpretation

| Threads | Time (ms) | Items/s    | vs 1-thread |
|---------|-----------|------------|-------------|
| 1       | 1.13      | 8.85 M/s   | 1.00×       |
| 2       | 0.86      | 11.69 M/s  | 1.32×       |
| 4       | 1.89      | 5.29 M/s   | 0.60×       |
| 8       | 4.72      | 2.12 M/s   | 0.24×       |

- **1 → 2 threads:** Modest gain (1.32×). Some parallelism helps, but contention
  already caps scaling well below the ideal 2×.
- **2 → 4 threads:** Throughput **drops** below the single-thread baseline.
  The single `mutex_` is saturated — threads spend more time waiting for the
  lock than executing.
- **4 → 8 threads:** Further collapse to 0.24× of baseline. Adding more
  contestants to the mutex fight only makes it worse.

### Reproduce

```bash
cmake --preset release -DTS_BUILD_BENCHMARKS=ON
cmake --build --preset release
./build/release/benchmarks/contention
```

## Per-Worker Contention Benchmark (`per_worker_contention`)

Measures the per-worker queue approach (`PerWorkerPool`) under the same conditions.
Expected to eliminate the global-mutex bottleneck seen above.

### Methodology

- **Pool:** `ts::PerWorkerPool` with 1, 2, 4, 8 worker threads.
- **Tasks:** 10,000 per iteration. Each spins for 0 µs (no-op + atomic increment)
  to expose pure lock contention.
- **Build:** Release (`-O2`), no sanitizers, real-time wall-clock measurement.
- **Tool:** Google Benchmark.

### Results

```
Benchmark                                       Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------
BM_PER_WORKER_CONTENTION/1/0/real_time       1.36 ms         1.29 ms          508 items_per_second=7.37744M/s
BM_PER_WORKER_CONTENTION/2/0/real_time      0.905 ms        0.837 ms          780 items_per_second=11.0554M/s
BM_PER_WORKER_CONTENTION/4/0/real_time       1.11 ms        0.999 ms          611 items_per_second=8.99452M/s
BM_PER_WORKER_CONTENTION/8/0/real_time       1.86 ms         1.75 ms          378 items_per_second=5.37983M/s
```

### Interpretation

| Threads | Time (ms) | Items/s   | vs 1-thread |
|---------|-----------|-----------|-------------|
| 1       | 1.36      | 7.38 M/s  | 1.00×       |
| 2       | 0.91      | 11.06 M/s | 1.50×       |
| 4       | 1.11      | 8.99 M/s  | 1.22×       |
| 8       | 1.86      | 5.38 M/s  | 0.73×       |

- **1 → 2 threads:** Decent gain (1.50×). Per-worker queues avoid the shared-mutex
  saturation that already caps `FixedThreadPool` at 1.32×.
- **2 → 4 threads:** Throughput holds (1.22× of baseline) instead of collapsing.
  The per-worker design absorbs the extra producers.
- **4 → 8 threads:** Some regression (0.73×) from the single-producer bottleneck —
  one thread round-robining across 8 queues can't keep all workers fed. Still
  far better than `FixedThreadPool`'s collapse to 0.24×.

### Comparison vs FixedThreadPool

| Threads | Contention | Per-Worker | Improvement |
|---------|-----------|------------|-------------|
| 1       | 1.13 ms   | 1.36 ms    | 0.83×       |
| 2       | 0.86 ms   | 0.91 ms    | 0.95×       |
| 4       | 1.89 ms   | 1.11 ms    | **1.70×**   |
| 8       | 4.72 ms   | 1.86 ms    | **2.54×**   |

At 1–2 threads the per-worker overhead (extra lock per `add_task`, hint dispatch)
makes it slightly slower. At 4+ threads the global-mutex bottleneck vanishes and
the design pulls ahead significantly.

### Lock Contention (`perf lock`)

```
$ sudo perf lock report -k contended -t
                Name   acquired  contended     avg wait   total wait     max wait     min wait 
     per_worker_cont         32         32      4.62 us    147.97 us     59.41 us      1.54 us 
          contention      96889      96889      4.06 us    393.57 ms     81.18 us      1.27 us 
```

| Metric              | Contention | Per-Worker | Reduction  |
|---------------------|-----------|------------|------------|
| Contended locks     | 96,889    | 32         | **~3,028×** |
| Total wait time     | 393.57 ms | 147.97 µs  | **~2,660×** |

`FixedThreadPool` spends **393 ms** of wall time waiting on its single mutex.
`PerWorkerPool` reduces this to **148 µs** — effectively zero contention.
The per-worker queues eliminate the single-mutex bottleneck entirely.

### Reproduce

```bash
cmake --preset release -DTS_BUILD_BENCHMARKS=ON
cmake --build --preset release
./build/release/benchmarks/per_worker_contention
```

### Upcoming Comparisons

| Stage | Approach              | Expected improvement vs baseline |
|-------|-----------------------|----------------------------------|
| 5     | Per-worker queues     | ✅ No lock contention at queue level; work imbalance possible |
| 6     | Per-worker + stealing | Imbalance mitigated; should approach ideal scaling |
