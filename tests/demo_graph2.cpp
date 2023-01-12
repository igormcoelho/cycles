
// C++
#include <map>
//
#include <demo_cptr/Graph.hpp>
#include <demo_cptr/XNode.hpp>

//
#include <cycles/cycles_ptr.hpp>
#include <cycles/detail/Tree.hpp>
#include <cycles/detail/utils.hpp>
#include <demo_cptr/MyGraph.hpp>
#include <pre-experiments/List.hpp>
#include <pre-experiments/nodes_exp.hpp>

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
    // G.entry = cycles_ptr<MyNode>(G.get_ctx(), new MyNode { .val = -1.0 });
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
    assert(G.my_ctx().lock()->forest.size() == 4);  // all independent
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 4);  // all independent
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 4);  // all independent
    G.print();
    std::cout << std::endl;
    std::cout << "WILL ADD LAST LINK" << std::endl;
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    std::cout << "forest size: " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 4);  // all independent
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
    // =======================
    // return 0;
    // OK, NO LEAKS UNTIL NOW!
    // =======================
    //
    // force reset: root dies but other refs still on main()
    G.entry.reset();
    assert(ptr3->neighbors[0]->val == -1);
    // force reset: node 2 still accessible through ptr1
    ptr2.reset();
    assert(ptr1->neighbors[0]->val == 2);
    // two root survivors
    std::cout << "forest size = " << G.my_ctx().lock()->forest.size()
              << std::endl;
    assert(G.my_ctx().lock()->forest.size() == 2);
    //
    auto& fake_ptr2 = ptr1->neighbors[0];
    auto& fake_entry = ptr3->neighbors[0];
    assert(G.entry.is_nullptr());
    assert(ptr1.is_root());
    assert(fake_ptr2.is_owned());  // node 2
    assert(ptr2.is_nullptr());
    assert(ptr3.is_root());
    assert(fake_entry.is_owned());  // node -1

    // deeper debug
    assert(ptr1.remote_node.lock()->has_parent() == false);
    assert(ptr1.remote_node.lock()->children.size() == 1);  // node 2
    assert(ptr1.remote_node.lock()->owned_by.size() == 1);  // node -1
    assert(ptr1.remote_node.lock()->owns.size() == 0);
    //
    fake_ptr2->val = 2.2;
    std::cout << "fake_ptr2 = "
              << fake_ptr2.remote_node.lock()->value_to_string() << std::endl;
    assert(fake_ptr2.remote_node.lock()->has_parent() == true);  // node 1
    assert(fake_ptr2.remote_node.lock()->children.size() == 0);
    assert(fake_ptr2.remote_node.lock()->owned_by.size() == 0);
    assert(fake_ptr2.remote_node.lock()->owns.size() == 1);  // node 3

    //
    assert(ptr3.remote_node.lock()->has_parent() == false);
    assert(ptr3.remote_node.lock()->children.size() == 1);  // node -1
    assert(ptr3.remote_node.lock()->owned_by.size() == 1);  // node 2
    assert(ptr3.remote_node.lock()->owns.size() == 0);
    //
    assert(fake_entry.remote_node.lock()->has_parent() == true);  // node 3
    assert(fake_entry.remote_node.lock()->children.size() == 0);
    assert(fake_entry.remote_node.lock()->owned_by.size() == 0);
    assert(fake_entry.remote_node.lock()->owns.size() == 1);  // node 1
    //
    // ptr3.reset(); // do not delete here
    // ptr1.reset(); // do not delete here
    //
    std::cout << "============================" << std::endl;
    std::cout << "BEGIN DEBUG FOR DESTRUCTION!" << std::endl;
    std::cout << "============================" << std::endl;
    G.entry.setDebug(true);
    ptr1.setDebug(true);
    ptr2.setDebug(true);
    ptr3.setDebug(true);
    G.my_ctx().lock()->debug = true;

    // MANUAL DESTRUCTION (NATURALLY OCCURRING IN THIS ORDER...)
    //
    ptr3.reset();

    // deeper debug
    assert(ptr1.remote_node.lock()->has_parent() == false);
    assert(ptr1.remote_node.lock()->children.size() == 1);  // node 2
    assert(ptr1.remote_node.lock()->owned_by.size() == 1);  // node -1
    assert(ptr1.remote_node.lock()->owns.size() == 0);
    //
    assert(fake_ptr2.remote_node.lock()->has_parent() == true);  // node 1
    assert(fake_ptr2.remote_node.lock()->children.size() == 1);  // node 3
    assert(fake_ptr2.remote_node.lock()->owned_by.size() == 0);
    assert(fake_ptr2.remote_node.lock()->owns.size() == 0);
    auto& fake_ptr3 = fake_ptr2.get().neighbors[0];
    //

    assert(fake_ptr3.remote_node.lock()->has_parent() == true);  // node 2
    assert(fake_ptr3.remote_node.lock()->children.size() == 1);  // node -1
    assert(fake_ptr3.remote_node.lock()->owned_by.size() == 0);
    assert(fake_ptr3.remote_node.lock()->owns.size() == 0);
    //
    assert(fake_entry.remote_node.lock()->has_parent() == true);  // node 3
    assert(fake_entry.remote_node.lock()->children.size() == 0);
    assert(fake_entry.remote_node.lock()->owned_by.size() == 0);
    assert(fake_entry.remote_node.lock()->owns.size() == 1);  // node 1
                                                              //
    // ptr1.reset(); // do not delete here
    //
    std::cout << "==================================" << std::endl;
    std::cout << "BEGIN DEBUG FOR FINAL DESTRUCTION!" << std::endl;
    std::cout << "==================================" << std::endl;

  }  // WILL LEAK... WHY?

  std::cout << "FINISHED!" << std::endl;
  return 0;
}