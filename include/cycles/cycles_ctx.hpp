// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_cycles_ctx_HPP_  // NOLINT
#define CYCLES_cycles_ctx_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <utility>
#include <vector>

//
#include <cycles/TNode.hpp>
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
  // pending deletions of nodes
  vector<NodeType> pending;

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

  bool is_destroying{false};

  void destroy_pending() {
    if (is_destroying) {
      std::cout << "WARNING: destroy_pending() already executing!" << std::endl;
      return;
    } else {
      std::cout << "CTX: NOT DESTROYING! BEGIN PROCESS!" << std::endl;
    }
    is_destroying = true;
    std::cout << "CTX: destroy_pending. |pending|=" << pending.size()
              << std::endl;
    if (pending.size() == 0) {
      is_destroying = false;
      return;
    }
    // TODO(igormcoelho): make queue?
    while (pending.size() > 0) {
      auto sptr_delete = std::move(pending[0]);
      pending.erase(pending.begin() + 0);
      // this must be clean, regarding external
      assert(sptr_delete->owned_by.size() == 0);
      // force clean owns list before continuing... should be good!
      for (unsigned i = 0; i < sptr_delete->owns.size(); i++) {
        auto sptr_owned = sptr_delete->owns[i].lock();
        TNodeHelper<sptr<T>>::removeFromOwnsList(sptr_delete, sptr_owned);
      }
      // is this guaranteed? must be...
      assert(sptr_delete->owns.size() == 0);
      // get its children
      auto children = std::move(sptr_delete->children);
      std::cout << "destroy_pending: destroy node" << std::endl;
      sptr_delete->debug_flag = true;
      // IMPORTANT: destroy node
      sptr_delete = nullptr;
      //
      std::cout << "destroy_pending: check children of node" << std::endl;
      // check if children can be saved
      while (children.size() > 0) {
        bool will_die = true;
        std::cout << "DEBUG: will move child!" << std::endl;
        auto sptr_child = std::move(children[0]);
        std::cout << "DEBUG: will erase empty child!" << std::endl;
        children.erase(children.begin() + 0);
        // I BELIEVE THAT, IN THIS CASE, ANY OWNER IS GOOD ENOUGH!
        // CHILD IS ROOT NOW, NO ONE IS ABOVE IT!
        // NO NEED TO CHECK DESCENDENT HERE!
        if (sptr_child->owned_by.size() > 0) {
          std::cout << "DEBUG: child found new parent!" << std::endl;
          will_die = false;
          auto sptr_new_parent = sptr_child->owned_by[0].lock();
          sptr_child->parent = sptr_new_parent;
          TNodeHelper<sptr<T>>::removeFromOwnsList(sptr_new_parent, sptr_child);
          TNodeHelper<sptr<T>>::removeFromOwnedByList(sptr_new_parent,
                                                      sptr_child);
          sptr_new_parent->add_child_strong(sptr_child);
        }
        // kill if not held by anyone now
        std::cout << "DEBUG: may kill child!" << std::endl;
        if (will_die) {
          pending.push_back(std::move(sptr_child));
          std::cout << "DEBUG: child sent to pending list! |pending|="
                    << pending.size() << std::endl;
        } else {
          std::cout << "DEBUG: child is saved!" << std::endl;
        }
      }  // while children exists
      //
    }  // while pending list
    std::cout << "destroy_pending: final clear pending list" << std::endl;
    pending.clear();
    is_destroying = false;
    std::cout << "destroy_pending: finished!" << std::endl;
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