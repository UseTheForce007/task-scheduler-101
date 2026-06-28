#include "ts/per_worker_pool.h"

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

namespace ts {

PerWorkerPool::PerWorkerPool(size_t num_workers) : wq_(num_workers)
{
	done_.store(false);
	for (size_t i = 0; i < num_workers; i++) {
		workers_.emplace_back(&PerWorkerPool::worker_loop, this, i);
	}
}

void
PerWorkerPool::add_task(std::function<void()> func)
{
	static thread_local size_t hint_ = 0;
	size_t idx = hint_++ % workers_.size();
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (done_) {
			lock.unlock();
			throw std::runtime_error(
					"Thread Pool is closed. Try again later");
		}
	}
	std::lock_guard<std::mutex> lock_(wq_[idx].mutex_);
	wq_[idx].queue_.push_back(func);
	// task_queue_.push(func);
	wq_[idx].cv_.notify_one();
}

PerWorkerPool::~PerWorkerPool()
{
	shutdown(DRAIN);
}

void
PerWorkerPool::shutdown(ShutdownPolicy policy)
{
	if (done_.exchange(true)) {
		return;
	}

	for (auto& [m, q, c] : wq_) {
		std::lock_guard<std::mutex> lock_(m);

		if (policy == CANCEL) {
			q = {};
		}

		c.notify_all();
	}

	for (auto& w : workers_) {
		if (w.joinable()) {
			w.join();
		}
	}
}

bool
PerWorkerPool::is_shutdown() const
{
	return done_.load();
}

void
PerWorkerPool::worker_loop(size_t id)
{
	while (true) {
		std::unique_lock<std::mutex> lock_(wq_[id].mutex_);
#ifdef TS_VERBOSE
		std::cout << "Worker " << id << " going to wait...\n";
#endif
		wq_[id].cv_.wait(lock_, [this, id] {
			return !wq_[id].queue_.empty() || done_;
		});
#ifdef TS_VERBOSE
		std::cout << "Worker " << id << " woke up! Queue empty? "
							<< wq_[id].queue_.empty() << "\n";
#endif
		if (done_ && wq_[id].queue_.empty()) {
			break;
		}
		auto task = std::move(wq_[id].queue_.front());
		wq_[id].queue_.pop_front();
		lock_.unlock();
#ifdef TS_VERBOSE
		std::cout << "Worker " << id << " executing task.\n";
		task();
		std::cout << "Worker " << id << " finished task.\n";
#else
		task();
#endif
	}
}

}	 // namespace ts
