
#include <map>

#include "Graph.hpp"
#include "XNode.hpp"
//
#include <cycles/List.hpp>
#include <cycles/Tree.hpp>
#include <cycles/cycle_ptr.hpp>
#include <cycles/nodes_exp.hpp>
#include <cycles/utils.hpp>

using std::string, std::vector, std::map;

int main()
{
  // graph part 2
  {
    std::cout << " -------- THIS PART CREATES MULTIPLES TREES -------- " << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "======== MyGraph ========" << std::endl;
    std::cout << std::endl;

    MyGraph<double> G;
    std::cout << "CONTEXT SHOULD NOT HAVE CREATED Tree for nullptr node" << std::endl;
    G.my_ctx().lock()->print();
    //
    G.print();
    //G.entry = cycle_ptr<MyNode>(G.get_ctx(), new MyNode { .val = -1.0 });
    std::cout << "WILL MAKE NODE -1" << std::endl;
    G.entry = G.make_node(-1.0);
    std::cout << "CONTEXT SHOULD HAVE CREATED Tree for -1 node" << std::endl;
    //
    std::cout << "FIRST PRINT!" << std::endl;
    G.print();
    // make cycle
    auto ptr1 = G.make_node(1.0);
    auto ptr2 = G.make_node(2.0);
    auto ptr3 = G.make_node(3.0);
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    //G.entry.get().neighbors.push_back(ptr1);
    std::cout << "---> setup G.entry" << std::endl;
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    //
    auto lsptr = G.my_ctx().lock();
    std::cout << "lsptr -> " << lsptr << std::endl;
    if (lsptr)
      lsptr->collect();
    //G.my_ctx().lock()->collect();
    std::cout << "FINAL PRINT!" << std::endl;
    G.print();
    std::cout << "forest size = " << G.my_ctx().lock()->forest.size() << std::endl;
  } // WILL LEAK

  std::cout << "FINISHED!" << std::endl;
  return 0;
}