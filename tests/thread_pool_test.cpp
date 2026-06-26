#include <gtest/gtest.h>
#include "ts/thread_pool.h"

TEST(ThreadPoolTest, Placeholder) {
  ts::FixedThreadPool pool{4};
  SUCCEED();
}
