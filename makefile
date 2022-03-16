all:  app_demo_sizeof app_demo_list app_demo_tree app_demo_graph1 app_demo_graph2 app_demo

app_demo: demo.cpp
	g++ demo.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo

app_demo_graph1: demo_graph1.cpp
	g++ demo_graph1.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo_graph1

app_demo_graph2: demo_graph2.cpp
	g++ demo_graph2.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo_graph2

app_demo_list: demo_list.cpp
	g++ demo_list.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo_list

app_demo_tree: demo_tree.cpp
	g++ demo_tree.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo_tree

app_demo_sizeof: demo_sizeof.cpp
	g++ demo_sizeof.cpp -Isrc/ -Wfatal-errors -g -std=c++17 -o build/app_demo_sizeof

graph1: app_demo_graph1
	@echo "."
	valgrind ./build/app_demo_graph1

graph2: app_demo_graph2
	@echo "."
	valgrind --leak-check=full ./build/app_demo_graph2


run: graph1 graph2


run_all: 
	@echo "."
	./build/app_demo_sizeof
	@echo "."
	./build/app_demo_list
	@echo "."
	./build/app_demo_graph1
	@echo "."
	./build/app_demo_graph2
	@echo "."
	./build/app_demo

clean:
	rm -f build/app_*
