all:   test_catch2  bazel_test # test_demo_graph2 

test_demo_graph2: demo_graph2.cpp
	g++ demo_graph2.cpp -I../include/ -I../src -g -std=c++17 -o ../build/test_demo_graph2
	valgrind ../build/test_demo_graph2

test_catch2:
	g++ MyGraph.Test.cpp MyList.Test.cpp TNode.Test.cpp -g --std=c++20 -I../include/ -I../src -Ithirdparty/ thirdparty/catch2/catch_amalgamated.cpp -o ../build/test_catch2
	valgrind --leak-check=full ../build/test_catch2

bazel_test:
	bazel test ...