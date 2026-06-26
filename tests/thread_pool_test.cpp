#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "ts/thread_pool.h"

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

TEST(ThreadPoolTest, TriggerDataRace)
{
	ts::FixedThreadPool pool(1);
	for (int i = 0; i < 1000; ++i) {
		pool.add_task([&] { std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
	}
}
