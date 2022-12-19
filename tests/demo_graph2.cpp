
// C++
#include <map>
//
#include <demo_cptr/Graph.hpp>
#include <demo_cptr/XNode.hpp>

//
#include <cycles/List.hpp>
#include <cycles/MyGraph.hpp>
#include <cycles/Tree.hpp>
#include <cycles/cycle_ptr.hpp>
#include <cycles/nodes_exp.hpp>
#include <cycles/utils.hpp>

using std::string, std::vector, std::map;

int main() {
  // graph part 2
  {
    std::cout << std::endl;
    std::cout
        << " -------- DEMO GRAPH2: THIS PART CREATES MULTIPLES TREES -------- "
        << std::endl;
    std::cout << std::endl;
    std::cout << "======== MyGraph ========" << std::endl;
    std::cout << std::endl;

    MyGraph<double> G;
    std::cout << "CONTEXT SHOULD NOT HAVE CREATED Tree for nullptr node"
              << std::endl;
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 0);
    G.my_ctx().lock()->print();
    //
    G.print();
    // G.entry = cycle_ptr<MyNode>(G.get_ctx(), new MyNode { .val = -1.0 });
    std::cout << "WILL MAKE NODE -1" << std::endl;
    G.entry = G.make_node(-1.0);
    std::cout << "CONTEXT SHOULD HAVE CREATED Tree for -1 node" << std::endl;
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 1);

    //
    std::cout << "FIRST PRINT!" << std::endl;
    G.print();
    // make cycle
    std::cout << "make_node 1.0" << std::endl;
    auto ptr1 = G.make_node(1.0);
    std::cout << "make_node 2.0" << std::endl;
    auto ptr2 = G.make_node(2.0);
    std::cout << "make_node 3.0" << std::endl;
    auto ptr3 = G.make_node(3.0);
    std::cout << "CONTEXT SHOULD HAVE CREATED Trees for nodes 1, 2 and 3"
              << std::endl;
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 4);
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    // G.entry.get().neighbors.push_back(ptr1);
    std::cout << "---> setup G.entry" << std::endl;
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 3);  // CASE 1.1A
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 3);  // CASE 2
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 2);  // CASE 1.1A
    G.print();
    std::cout << std::endl;
    std::cout << "WILL ADD LAST LINK" << std::endl;
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 2);  // CASE 2
    //
    std::cout << std::endl;
    std::cout << "after setup!" << std::endl;
    G.print();
    std::cout << "forest size = " << G.my_ctx().lock()->forest.size()
              << std::endl;
    //
    auto lsptr = G.my_ctx().lock();
    std::cout << "lsptr -> " << lsptr << std::endl;
    if (lsptr) lsptr->collect();
    // G.my_ctx().lock()->collect();
    std::cout << std::endl;
    std::cout << "FINAL PRINT!" << std::endl;
    G.print();
    std::cout << "forest size = " << G.my_ctx().lock()->forest.size()
              << std::endl;
    //
    ////G.entry = nullptr; // TODO: WHY THIS LEAKS??
  }  // WILL LEAK

  std::cout << "FINISHED!" << std::endl;
  return 0;
}