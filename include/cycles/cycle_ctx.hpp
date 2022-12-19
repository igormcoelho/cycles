// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_CYCLE_CTX_HPP_  // NOLINT
#define CYCLES_CYCLE_CTX_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <vector>

//
#include <cycles/Tree.hpp>
#include <cycles/utils.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// ========================
// cycle_ptr and cycle_ctx
// ========================
// smart pointer suitable for cycles
// memory is self-managed
//-------------------------

namespace cycles {

template <typename T>
// NOLINTNEXTLINE
class cycle_ctx {
 public:
  // collect strategy parameters
  bool auto_collect{true};

  // Forest management system comes here
  // using NodeType = sptr<TNode<T>>;
  // using TreeType = sptr<Tree<T>>;
  //
  using NodeType = sptr<TNode<sptr<T>>>;
  using TreeType = sptr<Tree<sptr<T>>>;

  // Forest: every Tree is identified by its Root node in map system
  map<NodeType, TreeType> forest;

 public:
  cycle_ctx() { std::cout << "cycle_ctx created!" << std::endl; }

  ~cycle_ctx() {
    std::cout << "~cycle_ctx() forest_size =" << forest.size() << std::endl;
    for (auto p : forest) {
      std::cout << " clearing root of ~> " << p.first << "'" << (*p.first)
                << "' -> " << p.second << " TREE" << std::endl;
      assert(p.second->root);  // root must never be nullptr
      std::cout << "   root.|children|=" << p.second->root->children.size()
                << std::endl;
      p.second->root->children.clear();  // clear children. IS THIS NECESSARY???
      p.second->root = nullptr;          // clear root
    }
    forest.clear();  // is it necessary??
  }

  void collect() {
    assert(this);
    // force collect
    std::cout << "collect! NOT Implemented (this=" << this << ")" << std::endl;
  }

  void print() {
    std::cout << "print ctx: (forest size=" << forest.size() << ") ["
              << std::endl;
    for (auto p : forest) {
      std::cout << " ~> ROOT_NODE " << p.first << " as '" << (*p.first)
                << "' -> TREE " << p.second << ": ";
      p.second->print();
    }
    std::cout << "]" << std::endl;
  }
};

}  // namespace cycles

#endif  // CYCLES_CYCLE_CTX_HPP_ // NOLINT