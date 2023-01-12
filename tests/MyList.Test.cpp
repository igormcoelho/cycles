
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
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
    REQUIRE(!L.my_ctx().lock()->debug);

    //
    L.entry = L.make_node(0);
    L.entry->next = L.make_node_owned(1, L.entry);
    L.entry->next->prev = L.entry.copy_owned(L.entry->next);
    auto& node1 = L.entry->next;
    //
    L.addNext(2, node1);
    auto& node2 = node1->next;
    L.addNext(3, node2);
    auto& node3 = node2->next;
    L.addNext(4, node3);
    auto& node4 = node3->next;
    // close cycle
    node4->next = L.entry.copy_owned(node4);
    L.entry->prev = node4.copy_owned(L.entry);
    //
    // CHECK CYCLE (FOUR TIMES) - next
    auto* ptr = &L.entry;
    for (int i = 0; i < 5 * 4; i++) {
      REQUIRE(ptr->get().val == i % 5);
      ptr = &ptr->get().next;
    }
    // CHECK CYCLE (FOUR TIMES) - prev
    ptr = &L.entry;
    for (int i = 5 * 4; i >= 0; i--) {
      REQUIRE(ptr->get().val == i % 5);
      ptr = &ptr->get().prev;
    }
    //
    auto p = L.my_ctx().lock()->debug_count_ownership_links();
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
    REQUIRE(p.first == 6);   // owns: 6 links
    REQUIRE(p.second == 6);  // owned_by: 6 links
  }
  // SHOULD NOT LEAK
  REQUIRE(mylistnode_count == 0);
}
