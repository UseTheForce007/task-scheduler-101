#pragma once

#include <cstddef>

namespace ts {

class FixedThreadPool {
 public:
  explicit FixedThreadPool(size_t num_workers);
  ~FixedThreadPool();

  FixedThreadPool(FixedThreadPool const &) = delete;
  FixedThreadPool &operator=(FixedThreadPool const &) = delete;

 private:
  void worker_loop(size_t id);
};

}  // namespace ts
