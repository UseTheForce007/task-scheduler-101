#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ts {

enum ShutdownPolicy
{
	DRAIN,
	CANCEL

};

class FixedThreadPool
{
   public:
	explicit FixedThreadPool(size_t num_workers);
	~FixedThreadPool();

	FixedThreadPool(FixedThreadPool const&) = delete;
	FixedThreadPool& operator=(FixedThreadPool const&) = delete;
	void add_task(std::function<void()>);
	void shutdown(ShutdownPolicy policy);
	bool is_shutdown() const;

   private:
	void worker_loop(size_t id);
	std::queue<std::function<void()>> task_queue_;
	std::vector<std::thread> workers_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::atomic_bool done_;
};

}  // namespace ts
