
#
add_executable(example1 basic_examples/app_example1.cpp)
target_link_libraries(example1 PRIVATE cycles)

add_executable(example2 basic_examples/app_example2.cpp)
target_link_libraries(example2 PRIVATE cycles)

add_executable(example3 basic_examples/app_example3.cpp)
target_link_libraries(example3 PRIVATE cycles)

add_executable(demo demo_cptr/demo.cpp)
target_link_libraries(demo PRIVATE cycles)

add_executable(demo0_cycles_test_graph demo_cptr/demo0_cycles_test_graph.cpp)
target_link_libraries(demo0_cycles_test_graph PRIVATE cycles)

