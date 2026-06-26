# ---------------------------------------------------------------------------
# Compiler options as an INTERFACE library so all targets get consistent flags
# ---------------------------------------------------------------------------

add_library(ts_compiler_options INTERFACE)
add_library(ts::compiler_options ALIAS ts_compiler_options)

target_compile_features(ts_compiler_options INTERFACE cxx_std_17)

target_compile_options(ts_compiler_options INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang>:
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  >
)

# Sanitizers — individually selectable so they don't conflict.
# TSan and ASan are mutually exclusive; each can pair with UBSan.
if(TS_ENABLE_TSAN)
  target_compile_options(ts_compiler_options INTERFACE
    -fsanitize=thread -fno-omit-frame-pointer -g -O1)
  target_link_options(ts_compiler_options INTERFACE
    -fsanitize=thread)
  message(STATUS "ThreadSanitizer enabled")
endif()

if(TS_ENABLE_ASAN)
  target_compile_options(ts_compiler_options INTERFACE
    -fsanitize=address -fno-omit-frame-pointer -g -O1)
  target_link_options(ts_compiler_options INTERFACE
    -fsanitize=address)
  message(STATUS "AddressSanitizer enabled")
endif()

if(TS_ENABLE_UBSAN)
  target_compile_options(ts_compiler_options INTERFACE
    -fsanitize=undefined -fno-omit-frame-pointer -g -O1)
  target_link_options(ts_compiler_options INTERFACE
    -fsanitize=undefined)
  message(STATUS "UndefinedBehaviourSanitizer enabled")
endif()
