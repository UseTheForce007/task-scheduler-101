# ---------------------------------------------------------------------------
# Reusable helpers for library, test, and benchmark targets
# ---------------------------------------------------------------------------

include(cmake/CompilerOptions.cmake)

# ---- library -----------------------------------------------------------
# ts_add_library(<name> source1 [source2 ...])
# Creates a static library linked against ts::compiler_options.
function(ts_add_library name)
  add_library(${name} STATIC ${ARGN})
  target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(${name} PUBLIC ts::compiler_options)
endfunction()

# ---- test --------------------------------------------------------------
# ts_add_test(<name> source1 [source2 ...])
# Creates a test executable linked against the named library and GTest.
function(ts_add_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name}
    ts
    GTest::GTest
    GTest::Main
    pthread
  )
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# ---- benchmark ---------------------------------------------------------
# ts_add_benchmark(<name> source1 [source2 ...])
# Creates a benchmark executable linked against the named library and
# Google Benchmark.
function(ts_add_benchmark name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name}
    ts
    benchmark::benchmark
    pthread
  )
endfunction()
