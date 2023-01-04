
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
#include <cycles/MyGraph.hpp>

using namespace std;     // NOLINT
using namespace cycles;  // NOLINT

// =======================
// memory management tests
// =======================

TEST_CASE("CyclesTestGraph: MyGraph A-B-C-D-E Simple") {
  std::cout << "begin MyGraph A-B-C-D-E Simple" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.is_root());
    //
    // begin make cycle
    //
    auto ptr1 = G.make_node(1.0);
    REQUIRE(ptr1.is_root());
    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(ptr2.is_root());
    //
    auto ptr3 = G.make_node(3.0);
    REQUIRE(ptr3.is_root());

    //
    // -1/entry -> 1 -> 2 -> 3 -> (-1/entry)
    //
    // CHECKS (B) - node 1 is owned by -1
    //
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(ptr1.is_root());
    REQUIRE(G.entry.get().neighbors[0].is_owned());
    REQUIRE(G.entry.get().neighbors[0].get().neighbors.size() == 0);
    //
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    //
    // CHECKS (C) - ptr1 is deleted
    //
    ptr1.reset();
    REQUIRE(ptr1.is_nullptr());
    //
    // CHECKS (D) - ptr2 and ptr3 are added as owners
    //
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    // CHECKS
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(G.entry.get().neighbors[0].remote_node.lock()->has_parent() ==
            true);
    REQUIRE(G.entry.get().neighbors[0].remote_node.lock()->children.size() ==
            0);
    REQUIRE(G.entry.get().neighbors[0].remote_node.lock()->owned_by.size() ==
            0);
    REQUIRE(G.entry.get().neighbors[0].remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr2.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr3.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr3.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr3.remote_node.lock()->owns.size() == 1);
    //
    // CHECKS (E) - ptr2 and ptr3 are removed
    //
    ptr2.reset();
    REQUIRE(ptr2.is_nullptr());
    //
    ptr3.reset();
    REQUIRE(ptr3.is_nullptr());
    //
    // FINALIZATION
    //
    REQUIRE(G.entry.get().val == -1);
    REQUIRE(G.entry->neighbors[0].get().val == 1);
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->val == 2);
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->neighbors[0]->val == 3);
    //
    // full cycle: -1 -> 1 -> 2 -> 3 -> -1
    REQUIRE(
        G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0]->val ==
        -1);
    // debug
    if (false) {
      G.entry->neighbors[0].setDebug(true);                              // 1
      G.entry->neighbors[0]->neighbors[0].setDebug(true);                // 2
      G.entry->neighbors[0]->neighbors[0]->neighbors[0].setDebug(true);  // 3
      G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0].setDebug(
          true);  // -1
    }
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}

// NOLINTNEXTLINE
TEST_CASE("CyclesTestGraph: MyGraph A-B-C-D-E Detailed") {
  std::cout << "begin MyGraph A-B-C-D-E Detailed" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);

    // context should not have created Tree for nullptr node
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    // creating -1 node

    G.entry = G.make_node(-1.0);

    // check few things on 'entry'... Parent, Children, Owned and Owns
    // CHECKS (A) - just -1 node
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 0);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);

    // forest size is 1
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    REQUIRE(G.entry.get_ref_use_count() == 2);
    REQUIRE(G.entry.is_root());
    //
    // make cycle

    auto ptr1 = G.make_node(1.0);
    REQUIRE(mynode_count == 2);
    REQUIRE(tnode_count == 2);
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    REQUIRE(ptr1.is_root());
    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(mynode_count == 3);
    REQUIRE(tnode_count == 3);
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    REQUIRE(ptr2.is_root());
    //
    auto ptr3 = G.make_node(3.0);
    REQUIRE(mynode_count == 4);
    REQUIRE(tnode_count == 4);
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    REQUIRE(ptr3.is_root());

    //
    // -1/entry -> 1 -> 2 -> 3 -> (-1/entry)
    //
    // copy of ptr1 will add weak link to owner (aka, G.entry), in owned_by
    // field
    // G.my_ctx().lock()->debug = true;
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    REQUIRE(ptr1.is_root());
    REQUIRE(G.entry.get().neighbors[0].is_owned());
    // CHECKS (B) - node 1 is owned by -1
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 0);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr1.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr1.remote_node.lock()->owns.size() == 0);

    //
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    REQUIRE(ptr1.is_root());
    REQUIRE(ptr1.get().neighbors[0].is_owned());
    // will destroy ptr1 from this context...
    // it still exists as G.entry.get().neighbors[0]
    ptr1.reset();
    REQUIRE(ptr1.is_nullptr());
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    REQUIRE(G.entry.get().neighbors[0].is_owned());
    auto& fake_ptr1 = G.entry.get().neighbors[0];
    // CHECKS (C) - ptr1 is deleted
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr2.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.remote_node.lock()->owns.size() == 0);
    //
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    //
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    // CHECKS (D) - ptr2 and ptr3 are added as owners
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr2.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr3.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr3.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr3.remote_node.lock()->owns.size() == 1);
    //
    // will clean all from this context
    //
    ptr2.reset();
    REQUIRE(ptr2.is_nullptr());
    ptr3.reset();
    REQUIRE(ptr3.is_nullptr());
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    auto& fake_ptr2 = fake_ptr1.get().neighbors[0];
    auto& fake_ptr3 = fake_ptr2.get().neighbors[0];
    // CHECKS (E) - ptr2 and ptr3 are removed
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.remote_node.lock()->children.size() == 1);
    REQUIRE(fake_ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr2.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr2.remote_node.lock()->children.size() == 1);
    REQUIRE(fake_ptr2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr2.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr3.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr3.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr3.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3.remote_node.lock()->owns.size() == 1);
    REQUIRE(fake_ptr3.remote_node.lock()->owns[0].lock().get() ==
            G.entry.remote_node.lock().get());
    //
    REQUIRE(G.entry.get().val == -1);
    REQUIRE(G.entry.count_owned_by() == 1);
    //
    REQUIRE(G.entry->neighbors[0].get().val == 1);
    REQUIRE(G.entry->neighbors[0].count_owned_by() == 0);
    REQUIRE(G.entry->neighbors[0].get().neighbors.size() == 1);
    //
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->val == 2);
    REQUIRE(G.entry->neighbors[0]->neighbors[0].count_owned_by() == 0);
    //
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->neighbors[0]->val == 3);
    REQUIRE(
        G.entry->neighbors[0]->neighbors[0]->neighbors[0].count_owned_by() ==
        0);
    //
    // full cycle: -1 -> 1 -> 2 -> 3 -> -1
    REQUIRE(
        G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0]->val ==
        -1);
    REQUIRE(G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                .count_owned_by() == 1);
    // try to take lock on owner
    REQUIRE((bool)G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                .getOwnedBy(0));
    // full cycle: -1 -> 1 -> 2 -> 3 -> -1 -> 1
    REQUIRE(G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->val == 1);
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    // make everyone verbose
    if (false) {
      G.entry->neighbors[0].setDebug(true);                              // -1
      G.entry->neighbors[0]->neighbors[0].setDebug(true);                // 1
      G.entry->neighbors[0]->neighbors[0]->neighbors[0].setDebug(true);  // 2
      G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0].setDebug(
          true);  // 3
    }
    //
    // manually invoke collection
    //
    auto lsptr = G.my_ctx().lock();
    if (lsptr) lsptr->collect();
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);  // NO COLLECTION??
    //
    // G.my_ctx().lock()->debug = true;
    // G.entry.setDebug(true);
    REQUIRE(true);
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}
