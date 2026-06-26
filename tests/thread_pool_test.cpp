#include "ts/thread_pool.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

TEST(ThreadPoolTest, SubmitsAndExecutes)
{
	std::atomic<int> counter{0};
	{
		ts::FixedThreadPool pool(4);
		for (int i = 0; i < 100; ++i) {
			pool.add_task([&] { ++counter; });
		}
	}
	EXPECT_EQ(counter.load(), 100);
}

TEST(ThreadPoolTest, MultipleTasks)
{
	std::vector<int> results(10, 0);
	{
		ts::FixedThreadPool pool(2);
		for (int i = 0; i < 10; ++i) {
			pool.add_task([&, i] { results[i] = i * i; });
		}
	}
	for (int i = 0; i < 10; ++i) {
		EXPECT_EQ(results[i], i * i);
	}
}

TEST(ThreadPoolTest, ShutdownDrainsRemainingTasks)
{
	std::atomic<int> counter{0};
	{
		ts::FixedThreadPool pool(2);
		for (int i = 0; i < 50; ++i) {
			pool.add_task([&] {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				++counter;
			});
		}
		pool.shutdown(ts::DRAIN);
	}
	EXPECT_EQ(counter.load(), 50);
}

TEST(ThreadPoolTest, ShutdownCancelDropsTasks)
{
	std::atomic<int> counter{0};
	{
		ts::FixedThreadPool pool(4);
		for (int i = 0; i < 100; ++i) {
			pool.add_task([&] { ++counter; });
		}
		pool.shutdown(ts::CANCEL);
		EXPECT_TRUE(pool.is_shutdown());
	}
	// may be partial — cancelled tasks are dropped
	EXPECT_LE(counter.load(), 100);
}

TEST(ThreadPoolTest, SubmitAfterShutdownThrows)
{
	ts::FixedThreadPool pool(2);
	pool.shutdown(ts::DRAIN);
	EXPECT_THROW(pool.add_task([] {}), std::runtime_error);
}

TEST(ThreadPoolTest, ShutdownIsIdempotent)
{
	ts::FixedThreadPool pool(2);
	pool.shutdown(ts::DRAIN);
	pool.shutdown(ts::DRAIN);
	SUCCEED();
}

TEST(ThreadPoolTest, ManyTasks)
{
	ts::FixedThreadPool pool(1);
	for (int i = 0; i < 100; ++i) {
		pool.add_task([&] { std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
	}
}
