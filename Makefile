all: test run_test demo

demo:
	cd src/demo_cptr && make

run_test:
	valgrind ./build/test_demo_graph2

test:
	cd tests && make