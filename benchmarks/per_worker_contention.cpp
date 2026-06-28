#include <benchmark/benchmark.h>

#include <atomic>
#include <chrono>

#include "ts/per_worker_pool.h"

static void
spin_us(long us)
{
	if (us == 0)
		return;
	auto start = std::chrono::steady_clock::now();
	while (std::chrono::duration_cast<std::chrono::microseconds>(
						 std::chrono::steady_clock::now() - start)
						 .count() < us) {
	}
}

static void

BM_PER_WORKER_CONTENTION(benchmark::State& state)
{
	int num_threads = state.range(0);
	int task_work_us = state.range(1);
	constexpr int num_tasks = 10000;
	for (auto _ : state) {
		ts::PerWorkerPool pool(num_threads);
		std::atomic_int counter{0};
		for (int i = 0; i < num_tasks; ++i) {
			pool.add_task([&counter, task_work_us] {
				spin_us(task_work_us);
				++counter;
			});
		}
		pool.shutdown(ts::DRAIN);
		if (counter.load() != num_tasks) {
			state.SkipWithError("Not all tasks completed");
			break;
		}
	}
	state.SetItemsProcessed(state.iterations() * num_tasks);
}

BENCHMARK(BM_PER_WORKER_CONTENTION)
		->ArgsProduct({benchmark::CreateRange(1, 8, 2), {0}})
		->Unit(benchmark::kMillisecond)
		->UseRealTime();

BENCHMARK_MAIN();
