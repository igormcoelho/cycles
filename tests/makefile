all:   test_catch2  bazel_test # test_demo_graph2 

test_demo_graph2: demo_graph2.cpp
	g++ demo_graph2.cpp -I../include/ -I../src -g -std=c++17 -o ../build/test_demo_graph2
	valgrind ../build/test_demo_graph2

test_catch2:
	g++ TNode.Test.cpp MyGraph.Test.cpp MyList.Test.cpp  -g --std=c++20 -I../include/ -I../src -Ithirdparty/ thirdparty/catch2/catch_amalgamated.cpp -o ../build/test_catch2
	valgrind --leak-check=full ../build/test_catch2

bench: quick_bench_sptr.cpp quick_bench_list_tree.cpp
	g++ quick_bench_sptr.cpp -Wfatal-errors   -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_sptr
	#
	g++ quick_bench_list_tree.cpp -Wfatal-errors  -DBENCH_LONG_DEFERRED -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_list_tree
	g++ quick_bench_list_tree.cpp -Wfatal-errors                        -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_list_tree_nodeferred
	#../build/bench_sptr
	# valgrind --leak-check=full ../build/bench_uptr
	echo ""
	echo "BENCHMARK - SMALL VALUES - ONLY RECURSIVE DESTRUCTORS"
	echo ""
	../build/bench_list_tree_nodeferred
	echo ""
	echo "BENCHMARK - BIG VALUES - ITERATIVE DESTRUCTORS"
	echo ""
	../build/bench_list_tree
	echo ""
	echo "BENCHMARK - SMALL VALUES - VALGRIND LEAK CHECKER"
	echo ""
	#
	valgrind --leak-check=full ../build/bench_list_tree_nodeferred




bazel_test:
	bazel test ...