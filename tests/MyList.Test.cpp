
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
//
// #include <cycles/detail/IDynowForest.hpp>
#include <cycles/relation_ptr.hpp>
#include <demo_cptr/MyList.hpp>

using namespace std;     // NOLINT
using namespace cycles;  // NOLINT

// =======================
// memory management tests
// =======================

TEST_CASE("CyclesTestMyList: MyList 5") {
  std::cout << "begin MyList 5" << std::endl;
  // create context
  {
    MyList L;
    // L.my_ctx().lock()->debug = true;
    REQUIRE(!L.my_ctx().lock()->debug());

    //
    L.entry = L.make_node(0);
    L.entry->next = L.make_node_owned(1, L.entry);
    L.entry->next->prev = L.entry.get_owned(L.entry->next);
    auto& node1 = L.entry->next;
    //
    L.addNext(2, node1);
    auto& node2 = node1->next;
    L.addNext(3, node2);
    auto& node3 = node2->next;
    L.addNext(4, node3);
    auto& node4 = node3->next;
    // close cycle
    node4->next = L.entry.get_owned(node4);
    L.entry->prev = node4.get_owned(L.entry);
    //
    // CHECK CYCLE (FOUR TIMES) - next
    auto* ptr = &L.entry;
    for (int i = 0; i < 5 * 4; i++) {
      REQUIRE(ptr->get()->val == i % 5);
      ptr = &ptr->get()->next;
    }
    // CHECK CYCLE (FOUR TIMES) - prev
    ptr = &L.entry;
    for (int i = 5 * 4; i >= 0; i--) {
      REQUIRE(ptr->get()->val == i % 5);
      ptr = &ptr->get()->prev;
    }
    //
    auto p = L.my_ctx().lock()->debug_count_ownership_links();
    //
    // auto* idyn = L.my_ctx().lock().get();
    // auto* v1dyn = (V1_DynowForest*)(idyn);  // NOLINT
    // auto p = v1dyn->debug_count_ownership_links();
    REQUIRE(p.first == p.second);  // IMPORTANT CHECK!
    //
    //  double => is strong link (parent link not shown)
    //  single -> is owned_by weak link (owns link not shown)
    //
    //  ||
    //  \/   =>     =>     =>     =>
    // ( 0 ) <- (1) <- (2) <- (3) <- (4)--
    //  ^ |                           ^  |
    //  | ----------------------------|  |
    //  ----------------------------------
    //
    // owns: 6 links
    REQUIRE(p.first == 6);
    // owned_by: 6 links
    REQUIRE(p.second == 6);
    //
    // deeper debug
    //
    REQUIRE(L.entry.arrow.is_root());
    REQUIRE(L.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(L.entry.arrow.remote_node.lock()->children.size() == 1);  // node 1
    REQUIRE(L.entry.arrow.remote_node.lock()->owned_by.size() ==
            2);  // node 1 and node 4 ??
    REQUIRE(L.entry.arrow.remote_node.lock()->owns.size() == 1);  // node 4
    //
    REQUIRE(L.entry->next.arrow.is_owned());
    REQUIRE(L.entry->next.arrow.remote_node.lock()->has_parent() ==
            true);  // node 0
    REQUIRE(L.entry->next.arrow.remote_node.lock()->children.size() ==
            1);  // node 2
    REQUIRE(L.entry->next.arrow.remote_node.lock()->owned_by.size() ==
            1);  // node 2
    REQUIRE(L.entry->next.arrow.remote_node.lock()->owns.size() ==
            1);  // node 0
                 //
    REQUIRE(node4.arrow.is_owned());
    REQUIRE(node4.arrow.remote_node.lock()->has_parent() == true);  // node 3
    REQUIRE(node4.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(node4.arrow.remote_node.lock()->owned_by.size() == 1);  // node 0
    REQUIRE(node4.arrow.remote_node.lock()->owns.size() ==
            2);  // node 3 and node 0
    //
    // std::cout << std::endl << "DESTRUCTION!" << std::endl;
    // L.my_ctx().lock()->debug = true;
  }
  // SHOULD NOT LEAK
  REQUIRE(mylistnode_count == 0);
}

TEST_CASE("CyclesTestMyList: MyList Single Cycle") {
  std::cout << "begin MyList Single Cycle" << std::endl;
  // create context
  {
    MyList L;
    // L.my_ctx().lock()->debug = true;
    REQUIRE(!L.my_ctx().lock()->debug());

    //
    L.entry = L.make_node(-1);
    L.entry->next = L.entry.get_owned(L.entry);
    L.entry->prev = L.entry.get_owned(L.entry);
    //
    // CHECK CYCLE (10 TIMES) - next
    auto* ptr = &L.entry;
    for (int i = 0; i < 10; i++) {
      REQUIRE(ptr->get()->val == -1);
      ptr = &ptr->get()->next;
    }
    // CHECK CYCLE (10 TIMES) - prev
    ptr = &L.entry;
    for (int i = 10; i > 0; i--) {
      REQUIRE(ptr->get()->val == -1);
      ptr = &ptr->get()->prev;
    }
    //
    // deeper debug
    //
    REQUIRE(L.entry.arrow.is_root());
    REQUIRE(L.entry.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(L.entry.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(L.entry.arrow.remote_node.lock()->owned_by.size() == 2);  // self?
    REQUIRE(L.entry.arrow.remote_node.lock()->owns.size() == 2);      // self?
    //
    REQUIRE(L.entry->next.arrow.is_owned());
    REQUIRE(L.entry->next.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(L.entry->next.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(L.entry->next.arrow.remote_node.lock()->owned_by.size() ==
            2);                                                         // self?
    REQUIRE(L.entry->next.arrow.remote_node.lock()->owns.size() == 2);  // self?
    //
    REQUIRE(L.entry->prev.arrow.is_owned());
    REQUIRE(L.entry->prev.arrow.remote_node.lock()->has_parent() == false);
    REQUIRE(L.entry->prev.arrow.remote_node.lock()->children.size() == 0);
    REQUIRE(L.entry->prev.arrow.remote_node.lock()->owned_by.size() ==
            2);                                                         // self?
    REQUIRE(L.entry->prev.arrow.remote_node.lock()->owns.size() == 2);  // self?

    //
    // std::cout << std::endl << "DESTRUCTION!" << std::endl;
    // L.my_ctx().lock()->debug = true;
  }
  // SHOULD NOT LEAK
  REQUIRE(mylistnode_count == 0);
}

TEST_CASE("CyclesTestMyList: MyList relation_ptr void derived") {
  std::cout << "begin MyList relation_ptr void derived" << std::endl;
  // create context
  {
    relation_pool<> pool;
    relation_ptr<void> ptr_base{nullptr, pool};
    ptr_base = relation_ptr<double>{new double{1}, pool};
    sptr<double> p =
        std::static_pointer_cast<double, void>(ptr_base.get_shared());
    REQUIRE(*p == 1);
  }
  // SHOULD NOT LEAK
}
