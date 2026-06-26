# task-scheduler-101

A study of the makings of a decent scheduler in C++.

## Prerequisites

- CMake 3.21+
- C++17 compiler (GCC 12+ or Clang 16+)
- Google Test (libgtest-dev)
- Google Benchmark (libbenchmark-dev) — optional, for benchmarks

## Presets

Three presets are defined in `CMakePresets.json`:

| Preset    | Build type | Sanitizers          | Use case                        |
|-----------|------------|---------------------|---------------------------------|
| `debug`   | Debug      | TSan + UBSan        | Thread-safety development       |
| `asan`    | Debug      | ASan + UBSan        | Memory-error debugging          |
| `release` | Release    | none                | Performance benchmarks          |

## Build & Test

```bash
# Configure
cmake --preset debug

# Build
cmake --build --preset debug

# Run tests
ctest --preset debug

# All three in one shot:
cmake --preset debug   && cmake --build --preset debug   && ctest --preset debug
cmake --preset asan    && cmake --build --preset asan    && ctest --preset asan
cmake --preset release && cmake --build --preset release && ctest --preset release
```

Build output lands in `build/<preset>/` (e.g. `build/debug/`).

## Compile Commands

`compile_commands.json` is generated automatically inside each build directory
(e.g. `build/debug/compile_commands.json`). Editors and tools like clangd,
clang-tidy, and vim-lsp pick it up.

## Sanitizers

All three can be toggled independently via CMake options:

| Option             | Flag                          |
|--------------------|-------------------------------|
| ThreadSanitizer    | `-DTS_ENABLE_TSAN=ON`         |
| AddressSanitizer   | `-DTS_ENABLE_ASAN=ON`         |
| UndefinedBehaviour | `-DTS_ENABLE_UBSAN=ON`        |

TSan and ASan are **mutually exclusive** — enable only one at a time. UBSan is
compatible with both.

To enable a specific sanitizer on any preset:

```bash
cmake -B build/custom -DCMAKE_BUILD_TYPE=Debug -DTS_ENABLE_TSAN=ON
cmake --build build/custom
```
