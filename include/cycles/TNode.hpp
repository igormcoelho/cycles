// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_TNODE_HPP_  // NOLINT
#define CYCLES_TNODE_HPP_  // NOLINT

// C++
#include <iostream>
#include <vector>
//
#include <cycles/utils.hpp>

using std::ostream, std::vector;  // NOLINT

// ============================
//   memory-managed Tree Node
// ============================
// Tree Node
// all memory is self-managed

namespace cycles {

static int tnode_count = 0;

template <typename T>
class TNode {
  //
 public:
  //
  T value;
  bool debug_flag{false};
  //
  // weak pointer to parent in tree (null if root)
  wptr<TNode<T>> parent;
  // strong pointer in children
  vector<sptr<TNode<T>>> children;
  // list of nodes that weakly own me
  vector<wptr<TNode<T>>> owned_by;
  //
  explicit TNode(T value, bool _debug_flag = false,
                 wptr<TNode<T>> _parent = wptr<TNode<T>>())
      : value{value}, debug_flag{_debug_flag}, parent{_parent} {
    tnode_count++;
    if (debug_flag)
      std::cout << "TNode tnode_count = " << tnode_count << std::endl;
  }

  virtual ~TNode() {
    if (debug_flag) std::cout << "~TNode(" << *value << ")" << std::endl;
    tnode_count--;
    if (debug_flag)
      std::cout << "  -> ~TNode tnode_count = " << tnode_count << std::endl;
  }

  // ============ FUNDAMENTAL PROPERTIES ============
  // has_parent and !has_parent

  // NOLINTNEXTLINE
  bool has_parent() const { return (bool)parent.lock(); }

  // get_child for traversal
  sptr<TNode> get_child(int i) { return children[i]; }

  // IMPORTANT!
  // This method would work better with a std::set or std::map on children
  bool has_child(wptr<TNode> target) {
    auto t_sptr = target.lock();
    if (!t_sptr) {
      assert(false);  // STRANGE...
      return false;
    }
    for (unsigned i = 0; i < children.size(); i++) {
      if (t_sptr == children[i]) return true;
    }
    return false;
  }

  auto add_child_strong(sptr<TNode> nxt) {
    // check if parent is set correctly
    assert(this == nxt->parent.lock().get());
    //
    children.push_back(nxt);
  }

  /*
    auto add_child_weak(wptr<TNode> nxt) {
      // check if parent is set correctly
      // assert(this == nxt.lock()->parent.lock().get());
      std::cout
          << "IMPORTANT! add_child_weak does not require parent to be REAL "
             "parent..."
             "it could be problem! solution: use two lists (for weak or strong)"
          << std::endl;
      //
      children.push_back(VarTNode{._node = nxt});
      children.at(children.size() - 1).node.reset();
    }
    */

  // false if not found, OR target is nullptr
  bool remove_child(TNode* target) {
    if (!target) return false;
    for (unsigned i = 0; i < children.size(); i++) {
      if (target == children[i].get()) {
        children.erase(children.begin() + i);
        return true;
      }
    }
    return false;
  }

  auto add_weak_link_owned(wptr<TNode> nxt) {
    // check if parent is set correctly
    owned_by.push_back(nxt);
  }

  /*
    auto set_child(int i, sptr<TNode> nxt) {
      children[i].node = nxt;
      children[i]._node.reset();
    }
    */
  /*
    //
    auto set_child_weak(int i, wptr<TNode> nxt) {
      children[i].node.reset();
      children[i]._node = nxt;
    }
  */
  auto get_value() { return value; }

  friend ostream& operator<<(ostream& os, const TNode& me) {
    os << "TNode(" << me.value << ")";
    return os;
  }
};

}  // namespace cycles

#endif  // CYCLES_TNODE_HPP_ // NOLINT
