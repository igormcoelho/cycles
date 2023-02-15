all:   test_catch2  bazel_test  test_quick_bench # test_demo_graph2 

test_demo_graph2: demo_graph2.cpp
	g++ demo_graph2.cpp -I../include/ -I../src -g -std=c++17 -DCYCLES_TEST -o ../build/test_demo_graph2
	valgrind ../build/test_demo_graph2

test_catch2:
	g++ TNode.Test.cpp MyGraph.Test.cpp MyList.Test.cpp  -g --std=c++17 -DCYCLES_TEST -I../include/ -I../src -Ithirdparty/ thirdparty/catch2/catch_amalgamated.cpp -DCYCLES_TOSTRING -o ../build/test_catch2 
	valgrind --leak-check=full ../build/test_catch2

test_quick_bench: bench_list_tree_build
	echo "HELPFUL SHORT BENCH... FOR LEAK CHECK IN TESTS!"
	@echo "================================================"
	@echo "BENCHMARK - SMALL VALUES - VALGRIND LEAK CHECKER"
	@echo "------------------------------------------------"
	#
	valgrind --leak-check=full --show-leak-kinds=all  ../build/bench_list_tree_nodeferred

bench: bench_sptr bench_list_tree bench_graph

bench_sptr:
	g++ bench/quick_bench_sptr.cpp -Wfatal-errors   -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_sptr
	../build/bench_sptr

bench_list_tree_build:
	g++ bench/quick_bench_list_tree.cpp -Wfatal-errors  -DBENCH_LONG_DEFERRED  -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_list_tree
	g++ bench/quick_bench_list_tree.cpp -Wfatal-errors                         -std=c++17 -g -Ofast -I../include/ -I../src -o ../build/bench_list_tree_nodeferred

bench_list_tree: bench_list_tree_build
	#
	# valgrind --leak-check=full ../build/bench_uptr
	#
	@echo "====================================================="
	@echo "BENCHMARK - SMALL VALUES - ONLY RECURSIVE DESTRUCTORS"
	@echo "-----------------------------------------------------"
	../build/bench_list_tree_nodeferred
	#
	@echo "=============================================="
	@echo "BENCHMARK - BIG VALUES - ITERATIVE DESTRUCTORS"
	@echo "----------------------------------------------"
	../build/bench_list_tree
	#
	@echo "================================================"
	@echo "BENCHMARK - SMALL VALUES - VALGRIND LEAK CHECKER"
	@echo "------------------------------------------------"
	#
	valgrind --leak-check=full ../build/bench_list_tree_nodeferred


bench_graph:
	# g++ bench/quick_bench_graph.cpp -std=c++17 -g -Ofast -I../include/ -Ithirdparty -I../src -o ../build/quick_bench_graph
	# valgrind --leak-check=full ../build/quick_bench_graph
	#
	g++ bench/long_bench_graph.cpp -std=c++17 -g -Ofast -I../include/ -Ithirdparty -Ithirdparty/hsutter-gcpp/submodules/gsl/include -I../src -o ../build/long_bench_graph
	../build/long_bench_graph
	valgrind --leak-check=full ../build/long_bench_graph
	

bazel_test:
	bazel test ...