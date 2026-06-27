#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
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
	template <typename F, typename... Args>
	auto submit(F&& f, Args&&... args)
		-> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
	{
		using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			[f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
				return std::invoke(std::move(f), std::move(args)...);
			});
		std::future<ReturnType> res = task->get_future();
		add_task([task]() { (*task)(); });

		cv_.notify_one();

		return res;
	};

   private:
	void worker_loop(size_t id);
	std::queue<std::function<void()>> task_queue_;
	std::vector<std::thread> workers_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::atomic_bool done_;
};

}  // namespace ts
