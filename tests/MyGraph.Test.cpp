
//#define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <catch2/catch_amalgamated.hpp>

// good
#include <cycles/MyGraph.hpp>

using namespace std;     // NOLINT
using namespace cycles;  // NOLINT

// =======================
// memory management tests
// =======================

TEST_CASE("CyclesTestGraph: MyGraph2") {
  // open context
  {
    MyGraph<double> G;
    // context should not have created Tree for nullptr node
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    // creating -1 node
    G.entry = G.make_node(-1.0);
    // forest size is 1
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    // ref_use_count=2 {MyNode(-1)}
    // make cycle
    auto ptr1 = G.make_node(1.0);
    // MyNode mynode_count=2
    // TNode tnode_count = 2
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    auto ptr2 = G.make_node(2.0);
    // MyNode mynode_count=3
    // TNode tnode_count = 3
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    //
    auto ptr3 = G.make_node(3.0);
    // MyNode mynode_count=4
    // TNode tnode_count = 4
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    //
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    // CASE 1: owner is root of some tree! Solution: OWNER will take it!
    // => CASE 1.1A - I'm also root, should remove this Tree from forest...
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);  // CASE 1.1A
    //
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);  // CASE 2
    //
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);  // CASE 1.1A
    //
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);  // CASE 2
    //
    // manually invoke collection
    //
    auto lsptr = G.my_ctx().lock();
    if (lsptr) lsptr->collect();
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);  // NO COLLECTION??
    //
  }  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
  // ~MyNode mynode_count=0
  // ~cycle_ptr: ref_use_count=0{NULL}
  // ~cycle_ptr: ref_use_count=0{NULL}
}
