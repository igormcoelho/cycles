
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#ifdef HEADER_ONLY
#include <catch2/catch_amalgamated.hpp>
#else
#include <catch2/catch_all.hpp>
#endif
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
    REQUIRE(!G.my_ctx().lock()->debug());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    // reset -1 node
    G.entry.reset();
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);

    if (false) {
      G.entry.arrow.setDebug(true);  // -1
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
    REQUIRE(!G.my_ctx().lock()->debug());
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.arrow.is_root());

    if (false) {
      G.entry.arrow.setDebug(true);  // -1
    }

    auto ptr1 = G.make_node(1.0);
    REQUIRE(ptr1.arrow.is_root());
    //
    // CHECKS (B) - node 1 is owned by -1
    //
    G.entry->neighbors.push_back(ptr1.get_owned(G.entry));
    REQUIRE(G.entry->neighbors[0].arrow.is_owned());

    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(ptr2.arrow.is_root());

    auto ptr3 = G.make_node(3.0);
    REQUIRE(ptr3.arrow.is_root());

    //
    // G.print();
    // std::cout << "WILL RESET ptr1" << std::endl;

    // CHECKS (C') - ptr1 is deleted (but no ownership is given to ptr2)
    ptr1.reset();

    //
    // CHECKS (D') - ptr2 and ptr3 are added as owners
    //
    ptr2->neighbors.push_back(ptr3.get_owned(ptr2));
    ptr3->neighbors.push_back(G.entry.get_owned(ptr3));

    REQUIRE(ptr2.arrow.is_root());
    REQUIRE(ptr3.arrow.is_root());
    REQUIRE(ptr2.get()->neighbors[0].arrow.is_owned());
    REQUIRE(ptr3.get()->neighbors[0].arrow.is_owned());

    REQUIRE(!G.entry.arrow.is_null());
    //
    // G.print();
    //
    if (false) {
      std::cout << "PRINT ptr2 (testing operator*):" << *ptr2 << std::endl;
      for (unsigned i = 0; i < ptr2->neighbors.size(); i++)
        std::cout << "  ptr2 MyNode neighbor i=" << i
                  << " => type: " << ptr2.get()->neighbors[i].arrow.getType()
                  << std::endl;
    }
    // node 2 should point to node 3
    REQUIRE(ptr2.arrow.remote_node.lock()->owned_by.size() == 0);  // no one
    REQUIRE(ptr2.arrow.remote_node.lock()->owns.size() == 1);      // 3
    REQUIRE(ptr3.arrow.remote_node.lock()->owned_by.size() == 1);  // 2
    REQUIRE(ptr3.arrow.remote_node.lock()->owns.size() == 1);      // -1

    // CHECKS (E') - ptr2 and ptr3 are removed
    //
    // std::cout << std::endl;
    // std::cout << std::endl << "WILL RESET ptr2" << std::endl << std::endl;
    ptr2.reset();
    // node 2 should not point to node 3 anymore
    REQUIRE(ptr3.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr3.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(ptr3.arrow.is_root());
    REQUIRE(ptr3.get()->neighbors[0].arrow.is_owned());
    //
    // std::cout << std::endl;
    // std::cout << std::endl << "WILL RESET ptr3" << std::endl << std::endl;
    ptr3.reset();

    //
    // FINALIZATION
    //
    REQUIRE(!G.entry.arrow.is_null());
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(G.entry.get()->val == -1);
    REQUIRE(G.entry->neighbors.size() == 1);

    //
    // debug
    if (false) {
      G.entry.arrow.setDebug(true);  // -1
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
    REQUIRE(!G.my_ctx().lock()->debug());

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.arrow.is_root());
    //
    // begin make cycle
    //
    auto ptr1 = G.make_node(1.0);
    REQUIRE(ptr1.arrow.is_root());
    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(ptr2.arrow.is_root());
    //
    auto ptr3 = G.make_node(3.0);
    REQUIRE(ptr3.arrow.is_root());

    //
    // -1/entry -> 1 -> 2 -> 3 -> (-1/entry)
    //
    // CHECKS (B) - node 1 is owned by -1
    //
    G.entry.get()->neighbors.push_back(ptr1.get_owned(G.entry));
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(G.entry.get()->neighbors[0].arrow.is_owned());
    REQUIRE(G.entry.get()->neighbors[0].get()->neighbors.size() == 0);
    //
    ptr1.get()->neighbors.push_back(ptr2.get_owned(ptr1));
    //
    // CHECKS (C) - ptr1 is deleted
    //
    ptr1.reset();
    REQUIRE(ptr1.arrow.is_null());
    //
    // CHECKS (D) - ptr2 and ptr3 are added as owners
    //
    ptr2.get()->neighbors.push_back(ptr3.get_owned(ptr2));
    ptr3.get()->neighbors.push_back(G.entry.get_owned(ptr3));
    // CHECKS
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(
        G.entry.get()->neighbors[0].arrow.remote_node.lock()->has_parent() ==
        true);
    REQUIRE(
        G.entry.get()->neighbors[0].arrow.remote_node.lock()->children.size() ==
        0);
    REQUIRE(
        G.entry.get()->neighbors[0].arrow.remote_node.lock()->owned_by.size() ==
        0);
    REQUIRE(G.entry.get()->neighbors[0].arrow.remote_node.lock()->owns.size() ==
            1);
    //
    REQUIRE(ptr2.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr3.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr3.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr3.arrow.remote_node.lock()->owns.size() == 1);
    //
    // CHECKS (E) - ptr2 and ptr3 are removed
    //
    ptr2.reset();
    REQUIRE(ptr2.arrow.is_null());
    //
    ptr3.reset();
    REQUIRE(ptr3.arrow.is_null());
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
      G.entry->neighbors[0].arrow.setDebug(true);                // 1
      G.entry->neighbors[0]->neighbors[0].arrow.setDebug(true);  // 2
      G.entry->neighbors[0]->neighbors[0]->neighbors[0].arrow.setDebug(
          true);  // 3
      G.entry->neighbors[0]
          ->neighbors[0]
          ->neighbors[0]
          ->neighbors[0]
          .arrow.setDebug(true);  // -1
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
    REQUIRE(!G.my_ctx().lock()->debug());
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;

    // context should not have created Tree for nullptr node
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    // creating -1 node

    G.entry = G.make_node(-1.0);
    REQUIRE(mynode_count == 1);
    REQUIRE(tnode_count == 1);

    // check few things on 'entry'... Parent, Children, Owned and Owns
    // CHECKS (A) - just -1 node
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 0);

    // forest size is 1
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    //
    auto entry_sptr = G.entry.get_shared();
    REQUIRE(entry_sptr.use_count() == 2);
    //
    REQUIRE(G.entry.arrow.is_root());
    //
    // make cycle

    auto ptr1 = G.make_node(1.0);
    REQUIRE(mynode_count == 2);
    REQUIRE(tnode_count == 2);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    REQUIRE(ptr1.arrow.is_root());
    //
    auto ptr2 = G.make_node(2.0);
    REQUIRE(mynode_count == 3);
    REQUIRE(tnode_count == 3);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 3);
    REQUIRE(ptr2.arrow.is_root());
    //
    auto ptr3 = G.make_node(3.0);
    REQUIRE(mynode_count == 4);
    REQUIRE(tnode_count == 4);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);
    REQUIRE(ptr3.arrow.is_root());

    //
    // -1/entry -> 1 -> 2 -> 3 -> (-1/entry)
    //
    // copy of ptr1 will add weak link to owner (aka, G.entry), in owned_by
    // field
    // G.my_ctx().lock()->debug = true;
    G.entry.get()->neighbors.push_back(ptr1.get_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(G.entry.get()->neighbors[0].arrow.is_owned());
    // CHECKS (B) - node 1 is owned by -1
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr1.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr1.arrow.remote_node.lock()->owns.size() == 0);

    //
    ptr1.get()->neighbors.push_back(ptr2.get_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(ptr1.get()->neighbors[0].arrow.is_owned());
    // will destroy ptr1 from this context...
    // it still exists as G.entry.get()->neighbors[0]
    ptr1.reset();
    REQUIRE(ptr1.arrow.is_null());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 3);
    REQUIRE(G.entry.get()->neighbors[0].arrow.is_owned());
    auto& fake_ptr1 = G.entry.get()->neighbors[0];
    // CHECKS (C) - ptr1 is deleted
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr2.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.arrow.remote_node.lock()->owns.size() == 0);
    //
    ptr2.get()->neighbors.push_back(ptr3.get_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 3);
    //
    ptr3.get()->neighbors.push_back(G.entry.get_owned(ptr3));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 3);
    // CHECKS (D) - ptr2 and ptr3 are added as owners
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr2.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr2.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr2.arrow.remote_node.lock()->owns.size() == 1);
    //
    REQUIRE(ptr3.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr3.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(ptr3.arrow.remote_node.lock()->owns.size() == 1);
    //
    // will clean all from this context
    //
    ptr2.reset();
    REQUIRE(ptr2.arrow.is_null());
    ptr3.reset();
    REQUIRE(ptr3.arrow.is_null());
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    auto& fake_ptr2 = fake_ptr1.get()->neighbors[0];
    auto& fake_ptr3 = fake_ptr2.get()->neighbors[0];
    // CHECKS (E) - ptr2 and ptr3 are removed
    REQUIRE(G.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(G.entry.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owned_by.size() == 1);
    REQUIRE(G.entry.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr1.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->children.size() == 1);
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_ptr3.arrow.remote_node.lock()->has_parent() == true);
    REQUIRE(fake_ptr3.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr3.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3.arrow.remote_node.lock()->owns.size() == 1);
    REQUIRE(fake_ptr3.arrow.remote_node.lock()->owns[0].lock().get() ==
            G.entry.arrow.remote_node.lock().get());
    //
    REQUIRE(G.entry.get()->val == -1);
    REQUIRE(G.entry.arrow.count_owned_by() == 1);
    //
    REQUIRE(G.entry->neighbors[0].get()->val == 1);
    REQUIRE(G.entry->neighbors[0].arrow.count_owned_by() == 0);
    REQUIRE(G.entry->neighbors[0].get()->neighbors.size() == 1);
    //
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->val == 2);
    REQUIRE(G.entry->neighbors[0]->neighbors[0].arrow.count_owned_by() == 0);
    //
    REQUIRE(G.entry->neighbors[0]->neighbors[0]->neighbors[0]->val == 3);
    REQUIRE(G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                .arrow.count_owned_by() == 0);
    //
    // full cycle: -1 -> 1 -> 2 -> 3 -> -1
    REQUIRE(
        G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0]->val ==
        -1);
    REQUIRE(G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                .arrow.count_owned_by() == 1);
    // try to take lock on owner
    REQUIRE((bool)G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                .arrow.getOwnedBy(0));
    // full cycle: -1 -> 1 -> 2 -> 3 -> -1 -> 1
    REQUIRE(G.entry->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->neighbors[0]
                ->val == 1);
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    // make everyone verbose
    if (false) {
      G.entry->neighbors[0].arrow.setDebug(true);                // -1
      G.entry->neighbors[0]->neighbors[0].arrow.setDebug(true);  // 1
      G.entry->neighbors[0]->neighbors[0]->neighbors[0].arrow.setDebug(
          true);  // 2
      G.entry->neighbors[0]
          ->neighbors[0]
          ->neighbors[0]
          ->neighbors[0]
          .arrow.setDebug(true);  // 3
    }
    //
    // manually invoke collection
    //
    auto lsptr = G.my_ctx().lock();
    if (lsptr) lsptr->collect();
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);  // NO COLLECTION??
    //
    // G.my_ctx().lock()->debug = true;
    // G.entry.arrow.setDebug(true);
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
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    auto ptr2 = G.make_node(2.0);
    auto ptr3 = G.make_node(3.0);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    G.entry.get()->neighbors.push_back(ptr1.get_owned(G.entry));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);  // all independent
    //
    ptr1.get()->neighbors.push_back(ptr2.get_owned(ptr1));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);  // all independent
    //
    ptr2.get()->neighbors.push_back(ptr3.get_owned(ptr2));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);  // all independent
    //
    ptr3.get()->neighbors.push_back(G.entry.get_owned(ptr3));
    REQUIRE(G.my_ctx().lock()->getForestSize() == 4);  // all independent
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
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    auto& fake_ptr2 = ptr1->neighbors[0];
    auto& fake_entry = ptr3->neighbors[0];
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_owned());  // node 2
    REQUIRE(ptr2.arrow.is_null());
    REQUIRE(ptr3.arrow.is_root());
    REQUIRE(fake_entry.arrow.is_owned());  // node -1

    // deeper debug
    REQUIRE(ptr1.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.arrow.remote_node.lock()->children.size() == 1);  // node 2
    REQUIRE(ptr1.arrow.remote_node.lock()->owned_by.size() == 1);  // node -1
    REQUIRE(ptr1.arrow.remote_node.lock()->owns.size() == 0);
    // change value to 2.2
    fake_ptr2->val = 2.2;
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->has_parent() ==
            true);  // node 1
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr2.arrow.remote_node.lock()->owns.size() == 1);  // node 3

    //
    REQUIRE(ptr3.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr3.arrow.remote_node.lock()->children.size() == 1);  // node -1
    REQUIRE(ptr3.arrow.remote_node.lock()->owned_by.size() == 1);  // node 2
    REQUIRE(ptr3.arrow.remote_node.lock()->owns.size() == 0);
    //
    REQUIRE(fake_entry.arrow.remote_node.lock()->has_parent() ==
            true);  // node 3
    REQUIRE(fake_entry.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry.arrow.remote_node.lock()->owns.size() == 1);  // node 1
    //
    // ptr3.reset(); // do not delete here
    // ptr1.reset(); // do not delete here
    //
    // G.entry.arrow.setDebug(true);
    // ptr1.arrow.setDebug(true);
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
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    ptr1->neighbors.push_back(G.make_node(2.0).get_owned(ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    fake_ptr2->neighbors.push_back(G.make_node(3.0).get_owned(fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.get_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_owned());
    REQUIRE(fake_ptr3.arrow.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.arrow.is_owned());  // node -1
    // KILL PART!
    REQUIRE(fake_ptr2.arrow.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // only one root survivor (1)
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    //
    REQUIRE(fake_ptr2.arrow.is_null());

    // deeper debug
    REQUIRE(ptr1.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.arrow.is_null());
    // fake_ptr3 is broken now
    // REQUIRE(fake_ptr3.arrow.is_null());
    // fake_entry is broken now
    // REQUIRE(fake_entry.arrow.is_null());
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
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    ptr1->neighbors.push_back(G.make_node(2.0).get_owned(ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    fake_ptr2->neighbors.push_back(G.make_node(3.0).get_owned(fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.get_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_owned());
    REQUIRE(fake_ptr3.arrow.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.arrow.is_owned());  // node -1
    // add node 4 saving 3 and -1
    auto ptr4 = G.make_node(4.0);
    ptr4->neighbors.push_back(fake_ptr3.get_owned(ptr4));
    auto& fake_ptr3_2 = ptr4->neighbors[0];
    //
    // KILL PART!
    // std::cout << std::endl << "KILL PART!" << std::endl;
    // node 2 must die
    // node 3 and node -1 must survive (held by node 4)
    REQUIRE(fake_ptr2.arrow.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // two root survivors (1) and (4) -> 3 -1
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_null());
    // fake_ptr3 now is bad pointer, inaccessible from fake_ptr2,
    // but fake_ptr3_2 is accessible from ptr4
    REQUIRE(fake_ptr3_2.arrow.is_owned());
    //
    // fake_entry now is bad pointer, inaccessible from fake_ptr3,
    // but fake_entry_2 is accessible from fake_ptr3_2
    auto& fake_entry_2 = fake_ptr3_2->neighbors[0];
    REQUIRE(fake_entry_2.arrow.is_owned());
    //
    REQUIRE(ptr4.arrow.is_root());
    //
    // deeper debug
    //
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(ptr1.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.arrow.is_null());
    // fake_ptr3 is broken now, but fake_ptr3_2 is good
    REQUIRE(fake_ptr3_2.arrow.is_owned());
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->has_parent() == true);  // 4
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->children.size() == 1);  // -1
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->owns.size() == 0);
    // fake_entry is broken now, but fake_entry_2 is good
    REQUIRE(fake_entry_2.arrow.is_owned());
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->has_parent() == true);  // 3
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->owns.size() == 0);
    REQUIRE(ptr4.arrow.is_root());
    REQUIRE(ptr4.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr4.arrow.remote_node.lock()->children.size() == 1);  // 3
    REQUIRE(ptr4.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr4.arrow.remote_node.lock()->owns.size() == 0);
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
    // get_owned

    MyGraph<double> G;
    //
    // G.debug_flag = true;
    // G.my_ctx().lock()->debug = true;
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    //
    G.entry = G.make_node(-1.0);
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);

    // make cycle
    auto ptr1 = G.make_node(1.0);
    // THE LINE BELOW IS DIFFERENT FROM TEST 7 TO TEST 8 (THIS TEST)
    ptr1->neighbors.push_back(G.make_node_owned(2.0, ptr1));
    auto& fake_ptr2 = ptr1->neighbors[0];
    // THE LINE BELOW IS DIFFERENT FROM TEST 7 TO TEST 8 (THIS TEST)
    fake_ptr2->neighbors.push_back(G.make_node_owned(3.0, fake_ptr2));
    auto& fake_ptr3 = fake_ptr2->neighbors[0];
    fake_ptr3->neighbors.push_back(G.entry.get_owned(fake_ptr3));

    // force reset: root dies but other refs still on main(), from ptr1
    G.entry.reset();
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_owned());
    REQUIRE(fake_ptr3.arrow.is_owned());
    auto& fake_entry = fake_ptr3->neighbors[0];
    REQUIRE(fake_entry.arrow.is_owned());  // node -1
    // add node 4 saving 3 and -1
    auto ptr4 = G.make_node(4.0);
    ptr4->neighbors.push_back(fake_ptr3.get_owned(ptr4));
    auto& fake_ptr3_2 = ptr4->neighbors[0];
    //
    // KILL PART!
    // std::cout << std::endl << "KILL PART!" << std::endl;
    // node 2 must die
    // node 3 and node -1 must survive (held by node 4)
    REQUIRE(fake_ptr2.arrow.is_owned());  // node 2
    // force reset: node 2 is killed together with node 3 and node -1
    fake_ptr2.reset();
    // two root survivors (1) and (4) -> 3 -1
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(fake_ptr2.arrow.is_null());
    // fake_ptr3 now is bad pointer, inaccessible from fake_ptr2,
    // but fake_ptr3_2 is accessible from ptr4
    REQUIRE(fake_ptr3_2.arrow.is_owned());
    //
    // fake_entry now is bad pointer, inaccessible from fake_ptr3,
    // but fake_entry_2 is accessible from fake_ptr3_2
    auto& fake_entry_2 = fake_ptr3_2->neighbors[0];
    REQUIRE(fake_entry_2.arrow.is_owned());
    //
    REQUIRE(ptr4.arrow.is_root());
    //
    // deeper debug
    //
    REQUIRE(ptr1.arrow.is_root());
    REQUIRE(ptr1.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr1.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr1.arrow.remote_node.lock()->owns.size() == 0);
    REQUIRE(fake_ptr2.arrow.is_null());
    // fake_ptr3 is broken now, but fake_ptr3_2 is good
    REQUIRE(fake_ptr3_2.arrow.is_owned());
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->has_parent() == true);  // 4
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->children.size() == 1);  // -1
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_ptr3_2.arrow.remote_node.lock()->owns.size() == 0);
    // fake_entry is broken now, but fake_entry_2 is good
    REQUIRE(fake_entry_2.arrow.is_owned());
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->has_parent() == true);  // 3
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(fake_entry_2.arrow.remote_node.lock()->owns.size() == 0);
    REQUIRE(ptr4.arrow.is_root());
    REQUIRE(ptr4.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(ptr4.arrow.remote_node.lock()->children.size() == 1);  // 3
    REQUIRE(ptr4.arrow.remote_node.lock()->owned_by.size() == 0);
    REQUIRE(ptr4.arrow.remote_node.lock()->owns.size() == 0);
    // SHOULD NOT LEAK
  }
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 9 - MyGraph MultiGraph") {
  std::cout << "begin MyGraph MultiGraph" << std::endl;
  // create context
  {
    MyGraph<double> G;
    REQUIRE(!G.my_ctx().lock()->debug());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);

    // STEP (A)
    // creating -1 node
    G.entry = G.make_node(-1.0);
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    //
    // make cycle
    auto ptr1 = G.make_node(1.0);
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    ptr1->neighbors.push_back(G.make_node_owned(2.0, ptr1));
    // auto& fake_ptr2 = ptr1->neighbors[0];
    //
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    REQUIRE(ptr1.arrow.is_root());
    // check first arc
    REQUIRE(ptr1->neighbors[0].arrow.is_owned());
    REQUIRE(ptr1->neighbors[0].arrow.is_owned_by_node);
    //
    // Multi-link between ptr1 and ptr2: (ptr1->ptr2), (ptr1->ptr2)
    ptr1->neighbors.push_back(ptr1->neighbors[0].get_owned(ptr1));
    // scope for two refs
    {
      // refs to multiple arcs of same node
      auto& fake_ptr2_0 = ptr1->neighbors[0];
      auto& fake_ptr2_1 = ptr1->neighbors[1];
      // check again the first arc (no corruption should happen)
      REQUIRE(fake_ptr2_0.arrow.is_owned_by_node);
      REQUIRE(fake_ptr2_0.arrow.is_owned());
      //
      // check properties of all smart pointers involved
      REQUIRE(ptr1.arrow.is_root());
      REQUIRE(fake_ptr2_0.arrow.is_owned());
      REQUIRE(fake_ptr2_1.arrow.is_owned());
      // reset first arc (child must not be wiped out)
      fake_ptr2_0.reset();
      REQUIRE(fake_ptr2_0.arrow.is_null());
      REQUIRE(fake_ptr2_1.arrow.is_owned());
    }
    // try three arcs now (remember that first arc neighbors[0] is now nullptr)
    ptr1->neighbors.push_back(ptr1->neighbors[1].get_owned(ptr1));
    ptr1->neighbors.push_back(ptr1->neighbors[1].get_owned(ptr1));
    // first is null, rest is owned
    REQUIRE(ptr1->neighbors.size() == 4);
    // roots are -1 and 1
    REQUIRE(G.my_ctx().lock()->getForestSize() == 2);
    //
    // std::cout << "will reset node -1" << std::endl;
    //
    // reset -1 node
    G.entry.reset();
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    // reset ptr1
    ptr1.reset();
    REQUIRE(ptr1.arrow.is_null());

    if (false) {
      G.entry.arrow.setDebug(true);  // -1
    }
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 10 - MyGraph unowned and self-owned") {
  std::cout << "begin MyGraph unowned and self-owned" << std::endl;
  // create context
  {
    MyGraph<double> G;
    // create unowned node
    G.entry = G.make_node(-1.0);
    // create copy of self-owned node
    auto entry2 = G.entry.get_owned(G.entry);
    // check self-descendency here (TODO move for TNodeHelper tests)
    REQUIRE(TNodeHelper<>::isDescendent(entry2.arrow.remote_node.lock(),
                                        G.entry.arrow.remote_node.lock()));
    //
    // THIS IS A SELF-LINK... SOME DISCUSSIONS BELOW TO UNDERSTAND ITS MEANING.
    //
    // MEANING of is_root():
    //  - data IS DIRECTLY REACHABLE (unowned)
    //
    // MEANING of is_owned():
    //  - data is reachable though the OWNER node...
    //     if OWNER disappears, data may be freed.
    //
    // MEANING of is_null():
    //  - no data is present or data is no longer reachable (or already
    //  collected).
    //

    // INTERPRETATION: This relation_ptr represents a loop arrow
    // from MyNode(G.entry) to MyNode(G.entry)... However, it does
    // not add anything else regarding reachability. As soon as
    // MyNode(G.entry) is no longer reachable (by some root path)
    // it WILL be collected, regardless of this loop link.
    // Note that this link cannot be "converted" to other link kind,
    // for example, from "A to A" to "root to A"... every link is
    // immutable on relation_ptr, so this SHOULD behave as some
    // sort of "weak pointer", that does not increase data liveness.

    // check value
    REQUIRE(G.entry->val == -1);
    REQUIRE(entry2->val == -1);
    // change value
    entry2->val = 2;
    // check value again
    REQUIRE(G.entry->val == 2);
    REQUIRE(entry2->val == 2);
    // check status
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(entry2.arrow.is_owned());
    // check forest structure
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    //
    // create one more self-owned reference
    auto entry3 = entry2.get_self_owned();
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(entry2.arrow.is_owned());
    REQUIRE(entry3.arrow.is_owned());
    // check forest structure
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    //
    // destroy unowned/root reference
    //
    G.entry.reset();
    //
    // 'entry2' is now unreachable (only way was through G.entry),
    //   so both 'entry2' and 'G.entry' should be null now.
    //
    // check forest structure
    REQUIRE(G.my_ctx().lock()->getForestSize() == 0);
    // all cleared up already
    REQUIRE(mynode_count == 0);

    // both should be null
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(entry2.arrow.is_null());
    REQUIRE(entry3.arrow.is_null());
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}

TEST_CASE("CyclesTestGraph: TEST_CASE 11 - MyGraph get_unowned") {
  std::cout << "begin MyGraph get_unowned" << std::endl;
  // create context
  {
    MyGraph<double> G;
    // create unowned node
    G.entry = G.make_node(-1.0);
    // create copy of unowned node
    auto entry2 = G.entry.get_unowned();

    REQUIRE(G.entry);  // exists
    REQUIRE(!entry2);  // does not exist

    // check value
    REQUIRE(G.entry->val == -1);
    // check status
    REQUIRE(G.entry.arrow.is_root());
    REQUIRE(entry2.arrow.is_null());
    // check forest structure
    REQUIRE(G.my_ctx().lock()->getForestSize() == 1);
    // create owned node and then make unowned copy
    {
      auto cptr1 = G.make_node(1.0);
      G.entry->neighbors.push_back(cptr1.get_owned(G.entry));
      REQUIRE(cptr1.arrow.is_root());
      REQUIRE(G.entry->neighbors[0].arrow.is_owned());
      // try to copy unowned of neighbor 0
      auto unowned1 = G.entry->neighbors[0].get_unowned();
      REQUIRE(unowned1.arrow.is_null());
      // drop cptr1
    }

    // try again with unowned1 (with cptr1 dead now)
    auto unowned1 = G.entry->neighbors[0].get_unowned();
    REQUIRE(unowned1.arrow.is_root());

    // try again (II) with unowned2 - but cannot have double unonowned (on v1)
    auto unowned2 = G.entry->neighbors[0].get_unowned();
    REQUIRE(unowned2.arrow.is_null());

    //
    // destroy unowned/root reference
    //
    G.entry.reset();

    // both should be null
    REQUIRE(G.entry.arrow.is_null());
    REQUIRE(entry2.arrow.is_null());
  }
  // SHOULD NOT LEAK
  REQUIRE(mynode_count == 0);
}
