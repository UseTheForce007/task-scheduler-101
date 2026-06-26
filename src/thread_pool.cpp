#include "ts/thread_pool.h"

#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

namespace ts {

FixedThreadPool::FixedThreadPool(size_t num_workers)
{
	done_.store(false);
	for (size_t i = 0; i < num_workers; i++) {
		workers_.emplace_back(&FixedThreadPool::worker_loop, this, i);
	};
}

void
FixedThreadPool::add_task(std::function<void()> func)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if (done_) {
		lock.unlock();
		throw std::runtime_error("Thread Pool is closed. Try again later");
	}
	task_queue_.push(func);
	cv_.notify_one();
}

FixedThreadPool::~FixedThreadPool()
{
	shutdown(DRAIN);
}

void
FixedThreadPool::shutdown(ShutdownPolicy policy)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (done_.load()) {
			return;
		}
		done_.store(true);
		if (policy == CANCEL) {
			task_queue_ = {};
		}
	}
	cv_.notify_all();
	for (auto& w : workers_) {
		if (w.joinable())
			w.join();
	};
}

bool
FixedThreadPool::is_shutdown() const
{
	return done_.load();
}

void
FixedThreadPool::worker_loop(size_t id)
{
	(void)id;
	while (true) {
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock, [this] { return !task_queue_.empty() || done_; });
		if (done_ && task_queue_.empty()) {
			break;
		}
		auto task = std::move(task_queue_.front());
		task_queue_.pop();
		lock.unlock();
		task();
	}
}

}  // namespace ts
