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
      assert(p.second->root);  // root must never be nullptr
      if (debug)
        std::cout
            << "CTX: move tree root node to garbage for deferred destruction"
            << std::endl;
      // ===============================
      // DO NOT DESTROY RECURSIVELY HERE
      // p.second->root = nullptr;  // clear root BEFORE CHILDREN
      //
      // force clean both lists: owned_by and owns
      bool b1 = TNodeHelper<sptr<T>>::cleanOwnsAndOwnedByLists(p.second->root);
      assert(b1);
      //
      // move to pending
      pending.push_back(std::move(p.second->root));
      p.second->root = nullptr;  // useless... just to make sure it's not here
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
    if (debug)
      std::cout << "~cycles_ctx: final cleanup on pending" << std::endl;
    assert(!is_destroying);
    collect();
    if (debug) std::cout << "~cycles_ctx: finished final collect" << std::endl;
  }

 public:
  std::pair<int, int> debug_count_ownership_links() {
    std::pair<int, int> p{0, 0};
    for (auto& t : forest) {
      auto p1 = debug_count_owns_owned_by(t.second->root);
      p.first += p1.first;
      p.second += p1.second;
    }
    return p;
  }

 private:
  std::pair<int, int> debug_count_owns_owned_by(sptr<TNode<sptr<T>>> node) {
    std::pair<int, int> p{0, 0};
    p.first += node->owns.size();
    p.second += node->owned_by.size();
    for (unsigned i = 0; i < node->children.size(); i++) {
      auto p1 = debug_count_owns_owned_by(node->children[i]);
      p.first += p1.first;
      p.second += p1.second;
    }
    return p;
  }

 public:
  void destroy_tree(sptr<TNode<sptr<T>>> sptr_mynode) {
    if (debug) std::cout << "destroy: will destroy my tree." << std::endl;
    // find my tree
    auto tree_it = this->forest.find(sptr_mynode);
    if (tree_it == this->forest.end()) {
      // ????
      std::cout << "ERROR! COULD NOT FIND MY TREE!" << std::endl;
      assert(false);
    } else {
      if (debug) {
        std::cout << " ~~~> OK FOUND MY TREE. Delete it." << std::endl;
      }
      // clear tree
      this->forest.erase(tree_it);

      if (debug) {
        std::cout << " ~~~> OK DELETED MY TREE." << std::endl;
      }
    }
  }

 private:
  bool is_destroying{false};

 public:
  void collect() {
    if (is_destroying) {
      if (debug)
        std::cout << "WARNING: collect() already executing!" << std::endl;
      return;
    } else {
      if (debug)
        std::cout << "CTX: NOT DESTROYING! BEGIN PROCESS!" << std::endl;
    }
    is_destroying = true;
    if (debug)
      std::cout << "CTX: destroy_pending. |pending|=" << pending.size()
                << std::endl;
    if (pending.size() == 0) {
      is_destroying = false;
      return;
    }
    // ==============================
    //    begin destruction process
    // ==============================
    //
    // store data separately for delayed destruction
    std::vector<sptr<T>> vdata;

    // TODO(igormcoelho): make queue?
    while (pending.size() > 0) {
      if (debug) {
        std::cout << std::endl;
        std::cout << "CTX: WHILE processing pending list. |pending|="
                  << pending.size() << std::endl;
      }
      NodeType sptr_delete = std::move(pending[0]);
      pending.erase(pending.begin() + 0);
      //
      if (debug) {
        std::cout << "CTX: sptr_delete is: " << sptr_delete->value_to_string()
                  << std::endl;
      }
      if (debug) {
        std::cout << "CTX: sptr_delete with these properties: ";
        std::cout << "node |owns|=" << sptr_delete->owns.size()
                  << " |owned_by|=" << sptr_delete->owned_by.size()
                  << std::endl;
      }
      // this must be clean, regarding external (Except for cycles, maybe...)
      if (sptr_delete->owned_by.size() > 0) {
        // assert(sptr_delete->owned_by.size() == 0);
        std::cout << "CTX WARNING: owned_by but dying... must be some cycle!"
                  << std::endl;
      }
      // force clean both lists: owned_by and owns
      bool b1 = TNodeHelper<sptr<T>>::cleanOwnsAndOwnedByLists(sptr_delete);
      assert(b1);
      //
      assert(sptr_delete->owned_by.size() == 0);
      assert(sptr_delete->owns.size() == 0);
      //
      // get its children
      auto children = std::move(sptr_delete->children);
      if (debug)
        std::cout << "destroy_pending: found |children|=" << children.size()
                  << std::endl;
      if (debug)
        std::cout << "destroy_pending: destroy node (move to vdata)"
                  << std::endl;
      if (debug) {
        sptr_delete->debug_flag = true;
      }
      if (debug) {
        std::cout << "CTX: will destroy EMPTY node: "
                  << sptr_delete->value_to_string() << std::endl;
      }
      // IMPORTANT: move data to vdata
      vdata.push_back(std::move(sptr_delete->value));
      // IMPORTANT: destroy node (without any data)
      sptr_delete = nullptr;
      //
      if (debug)
        std::cout << "destroy_pending: check children of node" << std::endl;
      // check if children can be saved
      while (children.size() > 0) {
        bool will_die = true;
        if (debug) std::cout << "DEBUG: will move child!" << std::endl;
        auto sptr_child = std::move(children[0]);
        if (debug)
          std::cout << "DEBUG: child is " << sptr_child->value_to_string()
                    << std::endl;
        if (debug) std::cout << "DEBUG: will erase empty child!" << std::endl;
        children.erase(children.begin() + 0);
        // I THINK THAT WE NEED TO CHECK isDescendent HERE BECAUSE MY CHILD
        // CANNOT OWN ME
        for (unsigned k = 0; k < sptr_child->owned_by.size(); k++) {
          if (debug) std::cout << "DEBUG: child found new parent!" << std::endl;
          auto sptr_new_parent = sptr_child->owned_by[k].lock();
          //
          if (debug)
            std::cout << "DEBUG: sptr_new_parent="
                      << sptr_new_parent->value_to_string() << std::endl;
          // NOTE: costly O(tree_size)=O(N) test in worst case for
          // 'isDescendent'
          bool _isDescendent =
              TNodeHelper<sptr<T>>::isDescendent(sptr_new_parent, sptr_child);
          //
          if (debug)
            std::cout << "DEBUG: isDescendent=" << _isDescendent << " k=" << k
                      << std::endl;
          if (_isDescendent) {
            if (debug)
              std::cout
                  << "CTX DEBUG: owned_by is already my descendent! Discard. "
                  << "Will try next k!"
                  << "k=" << k << std::endl;
            // k++
            continue;
          }
          will_die = false;
          //
          sptr_child->parent = sptr_new_parent;
          TNodeHelper<sptr<T>>::removeFromOwnsList(sptr_new_parent, sptr_child);
          TNodeHelper<sptr<T>>::removeFromOwnedByList(sptr_new_parent,
                                                      sptr_child);
          sptr_new_parent->add_child_strong(sptr_child);
        }
        // kill if not held by anyone now
        if (debug) std::cout << "DEBUG: may kill child!" << std::endl;
        if (will_die) {
          if (debug)
            std::cout << "DEBUG: child will be send to pending list: "
                      << sptr_child->value_to_string() << std::endl;
          //
          // force clean both lists: owned_by and owns
          bool b1 = TNodeHelper<sptr<T>>::cleanOwnsAndOwnedByLists(sptr_child);
          assert(b1);
          //
          pending.push_back(std::move(sptr_child));
          if (debug)
            std::cout << "DEBUG: child sent to pending list! |pending|="
                      << pending.size() << std::endl;
        } else {
          if (debug) std::cout << "DEBUG: child is saved!" << std::endl;
          if (debug) {
            std::cout << "CTX: child with these properties: ";
            std::cout << "node |owns|=" << sptr_child->owns.size()
                      << " |owned_by|=" << sptr_child->owned_by.size()
                      << std::endl;
          }
        }
      }  // while children exists
      //
    }  // while pending list > 0
    if (debug)
      std::cout << "destroy_pending: finished pending list |pending|="
                << pending.size() << std::endl;
    //
    if (debug)
      std::cout << "destroy_pending: final clear vdata. |vdata|="
                << vdata.size() << std::endl;
    vdata.clear();
    //
    if (debug)
      std::cout << "destroy_pending: assert no more pending. |pending|="
                << pending.size() << std::endl;
    // IF THIS FAILS, WE MAY NEED TO INTRODUCE ANOTHER WHILE LOOP HERE,
    // TO RESTART THE PROCESS, UNTIL WE FINISH WITH ZERO pending LIST.
    assert(pending.size() == 0);

    is_destroying = false;
    if (debug) std::cout << "destroy_pending: finished!" << std::endl;
  }

 public:
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