cmake_minimum_required(VERSION 3.22)
project(cycles-project LANGUAGES CXX VERSION 0.0.1)
#
# C++17 by default; presets may raise it (e.g. 'modules' preset uses C++23)
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set (CMAKE_CXX_STANDARD 17)
endif()
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
Include(FetchContent)

# hsutter/gcpp needs its own (old) GSL: git submodule update --init --recursive
option(CYCLES_BUILD_GCPP_BENCH "Build benchmarks that depend on hsutter/gcpp" OFF)

# ===================
# header only library
# -------------------
add_library(cycles INTERFACE)
target_include_directories(cycles INTERFACE include/)
#
add_subdirectory(examples)
#
enable_testing()
add_subdirectory(tests)
