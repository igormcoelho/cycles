#==========
#  TESTS
#----------
# add_subdirectory(tests/thirdparty/catch2/)
FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.16.1)
FetchContent_MakeAvailable(Catch2)
#
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
#
# same defines as Bazel tests (except HEADER_ONLY: Catch2 comes from FetchContent)
add_executable(my_graph_test MyGraph.Test.cpp)
target_link_libraries(my_graph_test PRIVATE cycles Catch2::Catch2WithMain)
target_compile_definitions(my_graph_test PRIVATE CYCLES_TEST CYCLES_TOSTRING)
catch_discover_tests(my_graph_test)
#
add_executable(my_list_test MyList.Test.cpp)
target_link_libraries(my_list_test PRIVATE cycles Catch2::Catch2WithMain)
target_compile_definitions(my_list_test PRIVATE CYCLES_TEST CYCLES_TOSTRING)
catch_discover_tests(my_list_test)
#
add_executable(tnode_test TNode.Test.cpp)
target_link_libraries(tnode_test PRIVATE cycles Catch2::Catch2WithMain)
target_compile_definitions(tnode_test PRIVATE CYCLES_TEST CYCLES_TOSTRING)
catch_discover_tests(tnode_test)


# MANUAL:
# target_link_libraries(my_test PUBLIC Catch2)
# enable_testing()
# add_test(NAME MeusTestes COMMAND my_test)


# ========
# bench

add_executable(quick_bench_list_tree bench/quick_bench_list_tree.cpp)
target_link_libraries(quick_bench_list_tree PRIVATE cycles)
#
# same bench with 10M-element lists (BIG VALUES - ITERATIVE DESTRUCTORS)
add_executable(quick_bench_list_tree_long bench/quick_bench_list_tree.cpp)
target_link_libraries(quick_bench_list_tree_long PRIVATE cycles)
target_compile_definitions(quick_bench_list_tree_long PRIVATE BENCH_LONG_DEFERRED)
#
add_executable(quick_bench_sptr bench/quick_bench_sptr.cpp)
target_link_libraries(quick_bench_sptr PRIVATE cycles)
#
# hsutter gcpp dependency (TestGraph.hpp includes gcpp)
#
if(CYCLES_BUILD_GCPP_BENCH)
  # gcpp only builds with its pinned GSL (span size is ptrdiff_t); a modern
  # system GSL (e.g., libmsgsl-dev) fails with narrowing errors in gpage.h
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hsutter-gcpp/submodules/gsl/include/gsl)
    message(FATAL_ERROR "CYCLES_BUILD_GCPP_BENCH needs gcpp and its GSL submodule: "
                        "git submodule update --init --recursive")
  endif()
  add_library(gcpp_thirdparty INTERFACE)
  target_include_directories(gcpp_thirdparty INTERFACE
    thirdparty/
    # gsl dependency
    thirdparty/hsutter-gcpp/submodules/gsl/include)
  # with GSL_UNENFORCED_ON_CONTRACT_VIOLATION, old GSL turns Expects() into
  # __builtin_assume, which clang flags in gsl/span (third-party noise)
  target_compile_options(gcpp_thirdparty INTERFACE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-assume>)
  #
  add_executable(long_bench_graph bench/long_bench_graph.cpp)
  target_link_libraries(long_bench_graph PRIVATE cycles gcpp_thirdparty)
  #
  add_executable(quick_bench_graph bench/quick_bench_graph.cpp)
  target_link_libraries(quick_bench_graph PRIVATE cycles gcpp_thirdparty)
endif()
