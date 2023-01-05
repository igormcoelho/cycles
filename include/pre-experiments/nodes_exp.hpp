#pragma once

// EXPERIMENTAL nodes

#include <cycles/utils.hpp>

// strong circular list
struct Node1 {
  // TODO: variant
  sptr<Node1> next;
  //
  double value;
  //
};

struct List1 {
  sptr<Node1> head;

  void print(int max = 10) {
    auto p = head;
    int i = 0;
    std::cout << "list: ";
    while (p) {
      i++;
      std::cout << p->value << " ";
      p = p->next;
      if (i > max) break;
    }
    std::cout << std::endl;
  }
};

// ==================

struct Node2_Variant {
  // TODO: variant
  std::variant<sptr<Node2_Variant>, wptr<Node2_Variant>> next;
  //
  double value;
  //
};

struct Node3 {
  // TODO: variant
  sptr<Node3> next;
  wptr<Node3> _next;
  //
  double value;
  //
};

// strong circular list
// similar to (with unique_ptr):
// https://www.youtube.com/watch?v=JfmTagWcqoE
//
struct Node4_Herb {
  // TODO: variant
  sptr<Node4_Herb> next;
  sptr<Node4_Herb>& head;
  //
  double value;
  //
  auto get_next() { return next ? next.get() : head.get(); }
  //
};
