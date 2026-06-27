ts_add_library(ts
  thread_pool.cpp
)

if(TS_VERBOSE)
  target_compile_definitions(ts PRIVATE TS_VERBOSE)
endif()
