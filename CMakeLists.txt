cmake_minimum_required(VERSION 3.21)
project(task-scheduler-101 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Ensure the version is strictly required (prevents decaying to an older standard)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- options -----------------------------------------------------------
option(TS_BUILD_TESTS      "Build tests"      ON)
option(TS_BUILD_BENCHMARKS "Build benchmarks" ON)

option(TS_VERBOSE     "Enable verbose worker logging" OFF)

option(TS_ENABLE_TSAN "Enable ThreadSanitizer"      OFF)
option(TS_ENABLE_ASAN "Enable AddressSanitizer"      OFF)
option(TS_ENABLE_UBSAN "Enable UndefinedBehaviourSanitizer" OFF)

# ---- modules -----------------------------------------------------------
include(cmake/Functions.cmake)

# ---- subdirectories ----------------------------------------------------
add_subdirectory(src)

if(TS_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  add_subdirectory(tests)
endif()

if(TS_BUILD_BENCHMARKS)
  find_package(benchmark REQUIRED)
  add_subdirectory(benchmarks)
endif()
