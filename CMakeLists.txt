cmake_minimum_required(VERSION 3.22)
project(cycles-project LANGUAGES CXX VERSION 0.0.1)
#
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
Include(FetchContent)

# ===================
# header only library
# -------------------
add_library(cycles INTERFACE)
target_include_directories(cycles INTERFACE include/)
#
add_subdirectory(examples)
#
add_subdirectory(tests)
