
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
#include <demo_cptr/MyGraph.hpp>

using namespace std;     // NOLINT
using namespace cycles;  // NOLINT

// =======================
// memory management tests
// =======================

TEST_CASE("CyclesTestGraph: TEST_CASE 1 - MyGraph Single") {
  std::cout << "begin MyGraph Single" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.is_root());
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    // reset -1 node
    G.entry.reset();
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);

    if (false) {
      G.entry.setDebug(true);  // -1
    }
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 2 - MyGraph A B C' D' E'") {
  std::cout << "begin MyGraph MyGraph A B C' D' E'" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.is_root());

    if (false) {
      G.entry.setDebug(true);  // -1
    }

    auto ptr1 = G.make_node(1.0);
    REQUIRE(ptr1.is_root());
    //
    // CHECKS (B) - node 1 is owned by -1
    //
    G.entry.get()->neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(G.entry.get()->neighbors[0].is_owned());

    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(ptr2.is_root());

    auto ptr3 = G.make_node(3.0);
    REQUIRE(ptr3.is_root());

    //
    // G.print();
    // std::cout << "WILL RESET ptr1" << std::endl;

    // CHECKS (C') - ptr1 is deleted (but no ownership is given to ptr2)
    ptr1.reset();

    //
    // CHECKS (D') - ptr2 and ptr3 are added as owners
    //
    ptr2.get()->neighbors.push_back(ptr3.copy_owned(ptr2));
    ptr3.get()->neighbors.push_back(G.entry.copy_owned(ptr3));

    REQUIRE(ptr2.is_root());
    REQUIRE(ptr3.is_root());
    REQUIRE(ptr2.get()->neighbors[0].is_owned());
    REQUIRE(ptr3.get()->neighbors[0].is_owned());

    REQUIRE(!G.entry.is_nullptr());
    //
    // G.print();
    //
    if (false) {
      std::cout << "PRINT ptr2 details:" << ptr2.get() << std::endl;
      for (unsigned i = 0; i < ptr2.get()->neighbors.size(); i++)
        std::cout << "  ptr2 MyNode neighbor i=" << i
                  << " => type: " << ptr2.get()->neighbors[i].getType()
                  << std::endl;
    }
    // node 2 should point to node 3
    REQUIRE(ptr2.remote_node.lock()->owned_by.size() == 0);  // no one
    REQUIRE(ptr2.remote_node.lock()->owns.size() == 1);      // 3
    REQUIRE(ptr3.remote_node.lock()->owned_by.size() == 1);  // 2
    REQUIRE(ptr3.remote_node.lock()->owns.size() == 1);      // -1

    // CHECKS (E') - ptr2 and ptr3 are removed
    //
    // std::cout << std::endl;
    // std::cout << std::endl << "WILL RESET ptr2" << std::endl << std::endl;
    ptr2.reset();
    // node 2 should not point to node 3 anymore
    REQUIRE(ptr3.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr3.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(G.entry.is_root());
    REQUIRE(ptr3.is_root());
    REQUIRE(ptr3.get()->neighbors[0].is_owned());
    //
    // std::cout << std::endl;
    // std::cout << std::endl << "WILL RESET ptr3" << std::endl << std::endl;
    ptr3.reset();

    //
    // FINALIZATION
    //
    REQUIRE(!G.entry.is_nullptr());
    REQUIRE(G.entry.is_root());
    REQUIRE(G.entry.get()->val == -1);
    REQUIRE(G.entry->neighbors.size() == 1);

    //
    // debug
    if (false) {
      G.entry.setDebug(true);  // -1
    }
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 3 - MyGraph A-B-C-D-E Simple") {
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
    G.entry.get()->neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(ptr1.is_root());
    REQUIRE(G.entry.get()->neighbors[0].is_owned());
    REQUIRE(G.entry.get()->neighbors[0].get()->neighbors.size() == 0);
    //
    ptr1.get()->neighbors.push_back(ptr2.copy_owned(ptr1));
    //
    // CHECKS (C) - ptr1 is deleted
    //
    ptr1.reset();
    REQUIRE(ptr1.is_nullptr());
    //
    // CHECKS (D) - ptr2 and ptr3 are added as owners
    //
    ptr2.get()->neighbors.push_back(ptr3.copy_owned(ptr2));
    ptr3.get()->neighbors.push_back(G.entry.copy_owned(ptr3));
    // CHECKS
    REQUIRE(G.entry.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(G.entry.get()->neighbors[0].remote_node.lock()->has_parent() ==
            true);
    REQUIRE(G.entry.get()->neighbors[0].remote_node.lock()->children.size() ==
            0);
    REQUIRE(G.entry.get()->neighbors[0].remote_node.lock()->owned_by.size() ==
            0);
    REQUIRE(G.entry.get()->neighbors[0].remote_node.lock()->owns.size() == 1);
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
    REQUIRE(G.entry.get()->val == -1);
    REQUIRE(G.entry->neighbors[0].get()->val == 1);
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
TEST_CASE("CyclesTestGraph: TEST_CASE 4 - MyGraph A-B-C-D-E Detailed") {
  std::cout << "begin MyGraph A-B-C-D-E Detailed" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;

    // context should not have created Tree for nullptr node
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    // creating -1 node

    G.entry = G.make_node(-1.0);
    REQUIRE(mynode_count == 1);
    REQUIRE(tnode_count == 1);

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
    G.entry.get()->neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    REQUIRE(ptr1.is_root());
    REQUIRE(G.entry.get()->neighbors[0].is_owned());
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
    ptr1.get()->neighbors.push_back(ptr2.copy_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    REQUIRE(ptr1.is_root());
    REQUIRE(ptr1.get()->neighbors[0].is_owned());
    // will destroy ptr1 from this context...
    // it still exists as G.entry.get()->neighbors[0]
    ptr1.reset();
    REQUIRE(ptr1.is_nullptr());
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    REQUIRE(G.entry.get()->neighbors[0].is_owned());
    auto& fake_ptr1 = G.entry.get()->neighbors[0];
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
    ptr2.get()->neighbors.push_back(ptr3.copy_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->forest.size() == 3);
    //
    ptr3.get()->neighbors.push_back(G.entry.copy_owned(ptr3));
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
    auto& fake_ptr2 = fake_ptr1.get()->neighbors[0];
    auto& fake_ptr3 = fake_ptr2.get()->neighbors[0];
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
    REQUIRE(G.entry.get()->val == -1);
    REQUIRE(G.entry.count_owned_by() == 1);
    //
    REQUIRE(G.entry->neighbors[0].get()->val == 1);
    REQUIRE(G.entry->neighbors[0].count_owned_by() == 0);
    REQUIRE(G.entry->neighbors[0].get()->neighbors.size() == 1);
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

// NOLINTNEXTLINE
TEST_CASE(
    "CyclesTestGraph: TEST_CASE 5 - MyGraph A-B-C-D force slow destruction") {
  std::cout << "begin MyGraph A-B-C-D force slow destruction" << std::endl;
  // create context
  {
    // THIS CASE FORCES GRAPH TO HAVE USELESS WEAK LINK ON TOP, UNTIL LAST
    // DESTRUCTION

    MyGraph<double> G;
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    auto ptr2 = G.make_node(2.0);
    auto ptr3 = G.make_node(3.0);
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    G.entry.get()->neighbors.push_back(ptr1.copy_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);  // all independent
    //
    ptr1.get()->neighbors.push_back(ptr2.copy_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);  // all independent
    //
    ptr2.get()->neighbors.push_back(ptr3.copy_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);  // all independent
    //
    ptr3.get()->neighbors.push_back(G.entry.copy_owned(ptr3));
    REQUIRE(G.my_ctx().lock()->forest.size() == 4);  // all independent
    //
    auto lsptr = G.my_ctx().lock();
    if (lsptr) lsptr->collect();
    // force reset: root dies but other refs still on main()
    G.entry.reset();
    REQUIRE(ptr3->neighbors[0]->val == -1);
    // force reset: node 2 still accessible through ptr1
    ptr2.reset();
    REQUIRE(ptr1->neighbors[0]->val == 2);
    // two root survivors
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    auto& fake_ptr2 = ptr1->neighbors[0];
    auto& fake_entry = ptr3->neighbors[0];
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_owned());  // node 2
    REQUIRE(ptr2.is_nullptr());
    REQUIRE(ptr3.is_root());
    REQUIRE(fake_entry.is_owned());  // node -1

    // deeper debug
    REQUIRE(ptr1.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.remote_node.lock()->children.size() == 1);  // node 2
    REQUIRE(ptr1.remote_node.lock()->owned_by.size() == 1);  // node -1
    REQUIRE(ptr1.remote_node.lock()->owns.size() == 0);
    // change value to 2.2
    fake_ptr2->val = 2.2;
    REQUIRE(fake_ptr2.remote_node.lock()->has_parent() == true);  // node 1
    REQUIRE(fake_ptr2.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr2.remote_node.lock()->owns.size() == 1);  // node 3

    //
    REQUIRE(ptr3.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.remote_node.lock()->children.size() == 1);  // node -1
    REQUIRE(ptr3.remote_node.lock()->owned_by.size() == 1);  // node 2
    REQUIRE(ptr3.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_entry.remote_node.lock()->has_parent() == true);  // node 3
    REQUIRE(fake_entry.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry.remote_node.lock()->owns.size() == 1);  // node 1
    //
    // ptr3.reset(); // do not delete here
    // ptr1.reset(); // do not delete here
    //
    // G.entry.setDebug(true);
    // ptr1.setDebug(true);
    //
    // std::cout << "will finish scope" << std::endl;
    // G.my_ctx().lock()->debug = true;

    // SHOULD NOT LEAK
  }
  REQUIRE(mynode_count == 0);
}

// NOLINTNEXTLINE
TEST_CASE("CyclesTestGraph: TEST_CASE 6 - MyGraph 1 2 3 -1 kill 2") {
  std::cout << "begin MyGraph 1 2 3 -1 kill 2" << std::endl;
  // create context
  {
    // THIS CASE FORCES GRAPH TO HAVE USELESS WEAK LINK ON TOP, UNTIL LAST
    // DESTRUCTION

    MyGraph<double> G;
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    ptr1->neighbors.push_back(G.make_node(2.0).copy_owned(ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    fake_ptr2->neighbors.push_back(G.make_node(3.0).copy_owned(fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.copy_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_owned());
    REQUIRE(fake_ptr3.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.is_owned());  // node -1
    // KILL PART!
    REQUIRE(fake_ptr2.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // only one root survivor (1)
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    //
    REQUIRE(fake_ptr2.is_nullptr());

    // deeper debug
    REQUIRE(ptr1.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.is_nullptr());
    // fake_ptr3 is broken now
    // REQUIRE(fake_ptr3.is_nullptr());
    // fake_entry is broken now
    // REQUIRE(fake_entry.is_nullptr());
    //
    // SHOULD NOT LEAK
  }
  REQUIRE(mynode_count == 0);
}

// NOLINTNEXTLINE
TEST_CASE(
    "CyclesTestGraph: TEST_CASE 7 - MyGraph 1 2 3 -1 (4) kill 2 but 4 saves 3 "
    "-1") {
  std::cout << "begin MyGraph 1 2 3 -1 (4) kill 2 but 4 saves 3 -1"
            << std::endl;
  // create context
  {
    // THIS CASE FORCES GRAPH TO HAVE USELESS WEAK LINK ON TOP, UNTIL LAST
    // DESTRUCTION

    MyGraph<double> G;
    //
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    ptr1->neighbors.push_back(G.make_node(2.0).copy_owned(ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    fake_ptr2->neighbors.push_back(G.make_node(3.0).copy_owned(fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.copy_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_owned());
    REQUIRE(fake_ptr3.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.is_owned());  // node -1
    // add node 4 saving 3 and -1
    auto ptr4 = G.make_node(4.0);
    ptr4->neighbors.push_back(fake_ptr3.copy_owned(ptr4));
    auto& fake_ptr3_2 = ptr4->neighbors[0];
    //
    // KILL PART!
    // std::cout << std::endl << "KILL PART!" << std::endl;
    // node 2 must die
    // node 3 and node -1 must survive (held by node 4)
    REQUIRE(fake_ptr2.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // two root survivors (1) and (4) -> 3 -1
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_nullptr());
    // fake_ptr3 now is bad pointer, inaccessible from fake_ptr2,
    // but fake_ptr3_2 is accessible from ptr4
    REQUIRE(fake_ptr3_2.is_owned());
    //
    // fake_entry now is bad pointer, inaccessible from fake_ptr3,
    // but fake_entry_2 is accessible from fake_ptr3_2
    auto& fake_entry_2 = fake_ptr3_2->neighbors[0];
    REQUIRE(fake_entry_2.is_owned());
    //
    REQUIRE(ptr4.is_root());
    //
    // deeper debug
    //
    REQUIRE(ptr1.is_root());
    REQUIRE(ptr1.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.is_nullptr());
    // fake_ptr3 is broken now, but fake_ptr3_2 is good
    REQUIRE(fake_ptr3_2.is_owned());
    REQUIRE(fake_ptr3_2.remote_node.lock()->has_parent() == true);  // 4
    REQUIRE(fake_ptr3_2.remote_node.lock()->children.size() == 1);  // -1
    REQUIRE(fake_ptr3_2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3_2.remote_node.lock()->owns.size() == 0);
    // fake_entry is broken now, but fake_entry_2 is good
    REQUIRE(fake_entry_2.is_owned());
    REQUIRE(fake_entry_2.remote_node.lock()->has_parent() == true);  // 3
    REQUIRE(fake_entry_2.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry_2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry_2.remote_node.lock()->owns.size() == 0);
    REQUIRE(ptr4.is_root());
    REQUIRE(ptr4.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr4.remote_node.lock()->children.size() == 1);  // 3
    REQUIRE(ptr4.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr4.remote_node.lock()->owns.size() == 0);
    // SHOULD NOT LEAK
  }
  REQUIRE(mynode_count == 0);
}

// NOLINTNEXTLINE
TEST_CASE(
    "CyclesTestGraph: TEST_CASE 8 - MyGraph 1 2 3 -1 (4) kill 2 but 4 saves 3 "
    "-1 with C2 constructor") {
  std::cout << "begin MyGraph 1 2 3 -1 (4) kill 2 but 4 saves 3 "
               "-1 with C2 constructor"
            << std::endl;
  // create context
  {
    // THIS CASE FORCES GRAPH TO HAVE USELESS WEAK LINK ON TOP, UNTIL LAST
    // DESTRUCTION
    //
    // THIS TEST 8 IS SAME AS TEST 7, USING make_node_owned INSTEAD OF
    // copy_owned

    MyGraph<double> G;
    //
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    // THE LINE BELOW IS DIFFERENT FROM TEST 7 TO TEST 8 (THIS TEST)
    ptr1->neighbors.push_back(G.make_node_owned(2.0, ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    // THE LINE BELOW IS DIFFERENT FROM TEST 7 TO TEST 8 (THIS TEST)
    fake_ptr2->neighbors.push_back(G.make_node_owned(3.0, fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.copy_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_owned());
    REQUIRE(fake_ptr3.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.is_owned());  // node -1
    // add node 4 saving 3 and -1
    auto ptr4 = G.make_node(4.0);
    ptr4->neighbors.push_back(fake_ptr3.copy_owned(ptr4));
    auto& fake_ptr3_2 = ptr4->neighbors[0];
    //
    // KILL PART!
    // std::cout << std::endl << "KILL PART!" << std::endl;
    // node 2 must die
    // node 3 and node -1 must survive (held by node 4)
    REQUIRE(fake_ptr2.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // two root survivors (1) and (4) -> 3 -1
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    REQUIRE(ptr1.is_root());
    REQUIRE(fake_ptr2.is_nullptr());
    // fake_ptr3 now is bad pointer, inaccessible from fake_ptr2,
    // but fake_ptr3_2 is accessible from ptr4
    REQUIRE(fake_ptr3_2.is_owned());
    //
    // fake_entry now is bad pointer, inaccessible from fake_ptr3,
    // but fake_entry_2 is accessible from fake_ptr3_2
    auto& fake_entry_2 = fake_ptr3_2->neighbors[0];
    REQUIRE(fake_entry_2.is_owned());
    //
    REQUIRE(ptr4.is_root());
    //
    // deeper debug
    //
    REQUIRE(ptr1.is_root());
    REQUIRE(ptr1.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.is_nullptr());
    // fake_ptr3 is broken now, but fake_ptr3_2 is good
    REQUIRE(fake_ptr3_2.is_owned());
    REQUIRE(fake_ptr3_2.remote_node.lock()->has_parent() == true);  // 4
    REQUIRE(fake_ptr3_2.remote_node.lock()->children.size() == 1);  // -1
    REQUIRE(fake_ptr3_2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3_2.remote_node.lock()->owns.size() == 0);
    // fake_entry is broken now, but fake_entry_2 is good
    REQUIRE(fake_entry_2.is_owned());
    REQUIRE(fake_entry_2.remote_node.lock()->has_parent() == true);  // 3
    REQUIRE(fake_entry_2.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry_2.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry_2.remote_node.lock()->owns.size() == 0);
    REQUIRE(ptr4.is_root());
    REQUIRE(ptr4.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr4.remote_node.lock()->children.size() == 1);  // 3
    REQUIRE(ptr4.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr4.remote_node.lock()->owns.size() == 0);
    // SHOULD NOT LEAK
  }
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 9 - MyGraph MultiGraph") {
  std::cout << "begin MyGraph MultiGraph" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug);
    REQUIRE(G.my_ctx().lock()->forest.size() == 0);

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.is_root());
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    //
    // make cycle
    auto ptr1 = G.make_node(1.0);
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    ptr1->neighbors.push_back(G.make_node_owned(2.0, ptr1));
    // auto& fake_ptr2 = ptr1->neighbors[0];
    //
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    REQUIRE(ptr1.is_root());
    // check first arc
    REQUIRE(ptr1->neighbors[0].is_owned());
    REQUIRE(ptr1->neighbors[0].is_owned_by_node);
    //
    // Multi-link between ptr1 and ptr2: (ptr1->ptr2), (ptr1->ptr2)
    ptr1->neighbors.push_back(ptr1->neighbors[0].copy_owned(ptr1));
    // scope for two refs
    {
      // refs to multiple arcs of same node
      auto& fake_ptr2_0 = ptr1->neighbors[0];
      auto& fake_ptr2_1 = ptr1->neighbors[1];
      // check again the first arc (no corruption should happen)
      REQUIRE(fake_ptr2_0.is_owned_by_node);
      REQUIRE(fake_ptr2_0.is_owned());
      //
      // check properties of all smart pointers involved
      REQUIRE(ptr1.is_root());
      REQUIRE(fake_ptr2_0.is_owned());
      REQUIRE(fake_ptr2_1.is_owned());
      // reset first arc (child must not be wiped out)
      fake_ptr2_0.reset();
      REQUIRE(fake_ptr2_0.is_nullptr());
      REQUIRE(fake_ptr2_1.is_owned());
    }
    // try three arcs now (remember that first arc neighbors[0] is now nullptr)
    ptr1->neighbors.push_back(ptr1->neighbors[1].copy_owned(ptr1));
    ptr1->neighbors.push_back(ptr1->neighbors[1].copy_owned(ptr1));
    // first is null, rest is owned
    REQUIRE(ptr1->neighbors.size() == 4);
    // roots are -1 and 1
    REQUIRE(G.my_ctx().lock()->forest.size() == 2);
    //
    std::cout << "will reset node -1" << std::endl;
    //
    // reset -1 node
    G.entry.reset();
    REQUIRE(G.entry.is_nullptr());
    REQUIRE(G.my_ctx().lock()->forest.size() == 1);
    // reset ptr1
    ptr1.reset();
    REQUIRE(ptr1.is_nullptr());

    if (false) {
      G.entry.setDebug(true);  // -1
    }
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}