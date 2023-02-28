
#==========
#  TESTS
#----------
# add_subdirectory(tests/thirdparty/catch2/)
FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.3.1)
FetchContent_MakeAvailable(Catch2)
#
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
#
add_executable(my_graph_test MyGraph.Test.cpp)
target_link_libraries(my_graph_test PRIVATE cycles Catch2::Catch2WithMain)
#
add_executable(my_list_test MyList.Test.cpp)
target_link_libraries(my_list_test PRIVATE cycles Catch2::Catch2WithMain)
#
add_compile_definitions(CYCLES_TEST)  # just for testing ?
catch_discover_tests(my_graph_test my_list_test)


# MANUAL:
# target_link_libraries(my_test PUBLIC Catch2)
# enable_testing()
# add_test(NAME MeusTestes COMMAND my_test)


# ======== 
# bench

add_executable(long_bench_graph bench/long_bench_graph.cpp)
target_link_libraries(long_bench_graph PRIVATE cycles)
#
add_executable(quick_bench_graph bench/quick_bench_graph.cpp)
target_link_libraries(quick_bench_graph PRIVATE cycles)
#
add_executable(quick_bench_list_tree bench/quick_bench_list_tree.cpp)
target_link_libraries(quick_bench_list_tree PRIVATE cycles)
#
# hsutter gcpp dependency
#
include_directories(thirdparty/)
# gsl dependency
include_directories(thirdparty/hsutter-gcpp/submodules/gsl/include)
#
add_executable(quick_bench_sptr bench/quick_bench_sptr.cpp)
target_link_libraries(quick_bench_sptr PRIVATE cycles)
