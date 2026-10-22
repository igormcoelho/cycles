all:  app_example1 app_example2 app_example3

app_example1: app_example1.cpp | ../../build
	g++ app_example1.cpp -I../../include/ -I../../examples/ -Wfatal-errors -g -std=c++17 -o ../../build/app_example1

app_example2: app_example2.cpp | ../../build
	g++ app_example2.cpp -I../../include/ -I../../examples/ -Wfatal-errors -g -std=c++17 -o ../../build/app_example2

app_example3: app_example3.cpp | ../../build
	g++ app_example3.cpp -I../../include/ -I../../examples/ -Wfatal-errors -g -std=c++17 -o ../../build/app_example3



run: app_example1 app_example2 app_example3
	valgrind --leak-check=full ../../build/app_example1
	valgrind --leak-check=full ../../build/app_example2
	valgrind --leak-check=full ../../build/app_example3

# regenerate the figures of app_example*.md (needs graphviz)
# color standard (white-filled nodes, readable on white and black):
#  - abstract graph figures (ex1_graph): blue #2f80ed edges
#  - data structure figures (*_structure): only purple and gray. Purple #9b59b6 for
#    pointers (including self), gray #888888 for back links (_prev); light purple
#    #efe3f7 entry boxes
#  - Dynow forest figures: red #d62728 solid = ownership, green #2ca02c dashed = weak,
#    gray #888888 = annotations (variable names)
svg:
	for f in figs/*.dot; do dot -Tsvg $$f -o $${f%.dot}.svg; done

clean:
	rm -f ../../build/app_*

# build/ is gitignored, so it doesn't exist in a fresh clone (e.g., CI)
../../build:
	mkdir -p $@
