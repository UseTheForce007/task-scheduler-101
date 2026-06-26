#include "ts/thread_pool.h"

#include <cstddef>

namespace ts {

FixedThreadPool::FixedThreadPool(size_t num_workers) {
  (void)num_workers;
}

FixedThreadPool::~FixedThreadPool() = default;

void FixedThreadPool::worker_loop(size_t id) {
  (void)id;
}

}  // namespace ts
