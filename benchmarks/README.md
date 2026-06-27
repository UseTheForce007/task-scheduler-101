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

### Upcoming Comparisons

| Stage | Approach              | Expected improvement vs baseline |
|-------|-----------------------|----------------------------------|
| 5     | Per-worker queues     | No lock contention at queue level; work imbalance possible |
| 6     | Per-worker + stealing | Imbalance mitigated; should approach ideal scaling |
