// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_TREE_HPP_  // NOLINT
#define CYCLES_TREE_HPP_  // NOLINT

#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TNodeV1.hpp>

using std::ostream;  // NOLINT

// =======================
//   memory-managed Tree
// =======================
// Tree
// all memory is self-managed

namespace cycles {

namespace detail {

// default is now type-erased T
template <typename T = TNodeData>
struct Tree {
  //
  sptr<TNode<T>> root;  // owned reference
  bool debug_flag{false};
  //
  // wptr<TNode<T>> tail_node; // DAG behavior, but... non-owning

  explicit Tree(bool _debug_flag = false)
      : root{nullptr}, debug_flag{_debug_flag} {
    // this->tail_node.reset();
  }

  void set_root(sptr<TNode<T>> _root) {
    this->root = _root;
    _root->parent = wptr<TNode<T>>();  // no parent on root node
    // //_root->tree_root = _root; // self-reference
  }

  // I Think... THIS WILL EXTEND the lifecycle of Data T, beyond limits of Tree
  // container
  sptr<T> get_root_data() {
    // See: https://www.youtube.com/watch?v=JfmTagWcqoE
    // => using aliasing constructor
    // "Point to That and manage with This"
    // managing lifetime by construction
    //
    // return {root, &(root->value)};
    //
    // simpler
    return root->value;
  }

  ~Tree() {
    if (debug_flag) {
      std::cout << "~Tree() => ";
      if (!this->root)
        std::cout << "nullptr" << std::endl;
      else
        std::cout << *this->root << std::endl;
    }

    // prevents stackoverflow on recursive destructor...
    // while (!empty())
    //  pop_front();
  }

  bool empty() { return !root; }

  // add_child at node_ptr. If 'node_ptr==nullptr', then this method is
  // 'set_root'
  // TODO: check if this method is consistent and necessary
  //
  void add_child(sptr<TNode<T>> node_ptr, T v) {
    // CHECK IF 'parent' field has been filled
    assert(this == node_ptr->parent);
    //
    // case n=0: initialize root
    if (this->root == nullptr) {
      //
      assert(node_ptr == nullptr);
      //
      this->root = sptr<TNode<T>>(new TNode<T>{v});
      // this->tail_node = this->head;
      // this->tail_node.lock()->set_next_weak(this->head); // circular
      return;
    }
    // case n>=1: general
    auto node = sptr<TNode<T>>(new TNode<T>{v});
    node_ptr.add(node);
    // this->tail_node.lock()->set_next_weak(this->head); // circular
  }

  void print() {
    std::cout << "Tree::print() => root (exists=" << (bool)this->root << ")"
              << std::endl;
    std::cout << "{";
    if (this->root) printFrom(this->root);
    std::cout << "}";
    std::cout << std::endl;
  }

  void printFrom(sptr<TNode<T>> node) {
    if (node) {
      std::cout << "node TNode<T>: {" << *node
                << "} |children|=" << node->children.size() << std::endl;
      if (node->children.size() > 0) {
        std::cout << "  => children: ";
        for (unsigned i = 0; i < node->children.size(); i++)
          std::cout << "{" << *node->get_child(i) << "}";
        std::cout << std::endl;
      }
      for (unsigned i = 0; i < node->children.size(); i++) {
        if (node->get_child(i) == this->root) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else {
          printFrom(node->get_child(i));
        }
      }
    }
  }
  //
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_TREE_HPP_ // NOLINT
