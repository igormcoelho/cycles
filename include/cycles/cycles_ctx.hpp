// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_cycles_ctx_HPP_  // NOLINT
#define CYCLES_cycles_ctx_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <vector>

//
#include <cycles/Tree.hpp>
#include <cycles/utils.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// ========================
// cycles_ptr and cycles_ctx
// ========================
// smart pointer suitable for cycles
// memory is self-managed
//-------------------------

namespace cycles {

template <typename T>
// NOLINTNEXTLINE
class cycles_ctx {
 public:
  // collect strategy parameters
  bool auto_collect{true};

  bool debug{false};

  // Forest management system comes here
  // using NodeType = sptr<TNode<T>>;
  // using TreeType = sptr<Tree<T>>;
  //
  using NodeType = sptr<TNode<sptr<T>>>;
  using TreeType = sptr<Tree<sptr<T>>>;

  // Forest: every Tree is identified by its Root node in map system
  map<NodeType, TreeType> forest;

 public:
  cycles_ctx() {
    if (debug) std::cout << "cycles_ctx created!" << std::endl;
  }

  ~cycles_ctx() {
    if (debug)
      std::cout << "~cycles_ctx() forest_size =" << forest.size() << std::endl;
    for (auto p : forest) {
      if (debug)
        std::cout << " clearing root of ~> " << p.first << "'" << (*p.first)
                  << "' -> " << p.second << " TREE" << std::endl;
      assert(p.second->root);    // root must never be nullptr
      p.second->root = nullptr;  // clear root BEFORE CHILDREN
      /*
      if (true)
        std::cout << "TODO: must remove weak links from root Tree node:"
                  << p.second->root->owned_by.size() << std::endl;
      if (debug)
        std::cout << " clearing children of Tree node:  root.|children|="
                  << p.second->root->children.size() << std::endl;
      p.second->root->children.clear();  // clear children. IS THIS NECESSARY???
      if (debug)
        std::cout << " clearing root with root = nullptr " << std::endl;
      p.second->root = nullptr;  // clear root
      */
    }
    if (debug) std::cout << "~cycles_ctx: final clear forest" << std::endl;
    forest.clear();  // is it necessary??
  }

  void collect() {
    assert(this);
    // force collect
    std::cout << "WARNING: collect! NOT Implemented (this=" << this << ")"
              << std::endl;
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

#endif  // CYCLES_cycles_ctx_HPP_ // NOLINT