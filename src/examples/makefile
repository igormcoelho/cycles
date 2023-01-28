all:  app_example1 app_example2

app_example1: app_example1.cpp
	g++ app_example1.cpp -I../../include/ -I../../src/ -Wfatal-errors -g -std=c++17 -o ../../build/app_example1

app_example2: app_example2.cpp
	g++ app_example2.cpp -I../../include/ -I../../src/ -Wfatal-errors -g -std=c++17 -o ../../build/app_example2


run: app_example1 app_example2
	valgrind --leak-check=full ../../build/app_example1
	valgrind --leak-check=full ../../build/app_example2


clean:
	rm -f ../../build/app_*
