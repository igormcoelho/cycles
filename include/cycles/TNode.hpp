// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_TNODE_HPP_  // NOLINT
#define CYCLES_TNODE_HPP_  // NOLINT

// C++
#include <iostream>
#include <vector>
// C++ HELPER ONLY
#include <memory>   // JUST FOR HELPER is_shared_ptr??
#include <sstream>  // just for value_to_string ??
#include <string>
//
#include <cycles/utils.hpp>

using std::ostream, std::vector;  // NOLINT

// ======================================
// Helper to detect if type is shared_ptr
//
template <class X>
struct is_shared_ptr : std::false_type {};
template <class X>
struct is_shared_ptr<std::shared_ptr<X>> : std::true_type {};
// ======================================

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
  // ===========================
  // => tree part
  // =========  WEAK  ==========
  // weak pointer to parent in tree (null if root)
  wptr<TNode<T>> parent;
  // ========= STRONG ==========
  // strong pointer in children
  vector<sptr<TNode<T>>> children;
  // ===========================
  // => non-tree part
  // =========  WEAK  ==========
  // list of nodes that weakly own me
  vector<wptr<TNode<T>>> owned_by;
  // list of nodes that I weakly own
  vector<wptr<TNode<T>>> owns;
  //
  explicit TNode(T value, bool _debug_flag = false,
                 wptr<TNode<T>> _parent = wptr<TNode<T>>())
      : value{value}, debug_flag{_debug_flag}, parent{_parent} {
    tnode_count++;
    if (debug_flag)
      std::cout << "TNode tnode_count = " << tnode_count << std::endl;
  }

  // helper!
  std::string value_to_string() {
    std::stringstream ss;
    if constexpr (is_shared_ptr<T>::value == true)
      ss << *value;
    else
      ss << value;
    return ss.str();
  }

  virtual ~TNode() {
    if (debug_flag) {
      std::cout << "BEGIN ~TNode(" << value_to_string() << ")" << std::endl;
    }
    tnode_count--;
    if (debug_flag) std::cout << "DEBUG: Part I will check" << std::endl;
    //
    // clear 'owns' list
    //
    if (owns.size() > 0) {
      if (debug_flag) {
        std::cout << "WARNING: inside ~TNode(" << value_to_string() << ")"
                  << std::endl;
        std::cout << "WARNING! PART I: must clean 'owns' list... not empty!"
                  << std::endl;
      }
      for (unsigned i = 0; i < owns.size(); i++) {
        auto s_owned_ptr = owns[i].lock();
        assert(s_owned_ptr);
        if (debug_flag) {
          std::cout << "OWNS i=" << i << " => "
                    << (s_owned_ptr->value_to_string()) << std::endl;
          std::cout << " i has owned_by count: " << s_owned_ptr->owned_by.size()
                    << std::endl;
        }
        // ==================
        assert(s_owned_ptr->owned_by.size() > 0);
        bool removed = false;
        for (int j = 0; j < s_owned_ptr->owned_by.size(); j++) {
          auto s_owner = s_owned_ptr->owned_by[j].lock();
          if (!s_owner || (s_owner && (this == s_owner.get()))) {
            // IMPORTANT: THIS IS MY DESTRUCTOR.. SHARED POINTERS SHOULD NOT
            // HOLD REFERENCES TO ME AT THIS POINT!
            // IF USING RAW POINTER WITH UNIQUE_PTR, EQUALITY WITH this SHOULD
            // STILL WORK... BUT WITH WEAK PTR, WE JUST ASSUME EMPTY IS ME.
            //
            s_owned_ptr->owned_by.erase(s_owned_ptr->owned_by.begin() + j);
            removed = true;
            if (debug_flag)
              std::cout << "PART I: REMOVED! j=" << j << std::endl;
            break;  // single time only (TODO: investigate...)
          }
        }  // for j
        assert(removed);
      }  // for i
      owns.clear();
      if (debug_flag)
        std::cout << "DEBUG: Part I finished cleanup owns list" << std::endl;
    }  // if owns exists
    if (debug_flag) std::cout << "DEBUG: Part II will check" << std::endl;
    //
    // clear 'owned_by' list
    //
    if (owned_by.size() > 0) {
      if (debug_flag) {
        std::cout << "PASSED IF... owned_by > 0" << std::endl;
        std::cout
            << "WARNING! PART II: must clean 'owned_by' list... not empty!"
            << std::endl;
      }
      for (unsigned i = 0; i < owned_by.size(); i++) {
        auto s_owner_ptr = owned_by[i].lock();
        assert(s_owner_ptr);
        if (debug_flag) {
          std::cout << "OWNER i=" << i << " => "
                    << (s_owner_ptr->value_to_string()) << std::endl;
          std::cout << " i is owner of count: " << s_owner_ptr->owns.size()
                    << std::endl;
        }
        assert(s_owner_ptr->owns.size() > 0);
        bool removed = false;
        for (int j = 0; j < s_owner_ptr->owns.size(); j++) {
          auto s_owned = s_owner_ptr->owns[j].lock();
          if (!s_owned || (s_owned && (this == s_owned.get()))) {
            // IMPORTANT: THIS IS MY DESTRUCTOR.. SHARED POINTERS SHOULD NOT
            // HOLD REFERENCES TO ME AT THIS POINT!
            // IF USING RAW POINTER WITH UNIQUE_PTR, EQUALITY WITH this SHOULD
            // STILL WORK... BUT WITH WEAK PTR, WE JUST ASSUME EMPTY IS ME.
            //
            s_owner_ptr->owns.erase(s_owner_ptr->owns.begin() + j);
            removed = true;
            if (debug_flag)
              std::cout << "PART II: REMOVED! j=" << j << std::endl;
            break;  // single time only (TODO: investigate...)
          }
        }  // for j
        assert(removed);
      }  // for i
      owned_by.clear();
      if (debug_flag) {
        std::cout << "DEBUG: Part I finished cleanup owned_by list"
                  << std::endl;
      }
    }  // if owned_by exists
    if (debug_flag)
      std::cout << "  -> ~TNode tnode_count = " << tnode_count << std::endl;
    if (debug_flag)
      std::cout << "FINISH ~TNode(" << value_to_string() << ")" << std::endl;
    if (debug_flag)
      std::cout << "FINISH ~TNode Will destroy value: " << value_to_string()
                << std::endl;
    // ONLY FOR SHARED POINTERS!
    if constexpr (is_shared_ptr<T>::value == true) {
      value = nullptr;
    } else {
      // CANNOT FORCE DESTRUCTION!
      if (debug_flag)
        std::cout << "FINISH REAL ~TNode WARNING: CANNOT FORCE DESTRUCTION!"
                  << std::endl;
    }

    if (debug_flag) std::cout << "FINISH REAL ~TNode(nullptr)" << std::endl;
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
             "it could be problem! solution: use two lists (for weak or
    strong)"
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

  static void add_weak_link_owned(sptr<TNode> who_is_owned,
                                  sptr<TNode> who_owns) {
    assert(who_is_owned);
    assert(who_owns);
    if (false) std::cout << "add_weak_link_owned:" << std::endl;
    // TODO(igormcoelho): check if parent is set correctly
    who_is_owned->owned_by.push_back(who_owns);
    who_owns->owns.push_back(who_is_owned);
    if (false) {
      std::cout << "\twho_is_owned:" << *who_is_owned->value
                << " |owns|=" << who_is_owned->owns.size()
                << " |owned|=" << who_is_owned->owned_by.size() << std::endl;
      std::cout << "\twho_owns:" << *who_owns->value
                << " |owns|=" << who_owns->owns.size()
                << " |owned|=" << who_owns->owned_by.size() << std::endl;
    }
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

template <typename T>
class TNodeHelper {
 public:
  // ================================================================
  // Check if 'myNewParent' is not my descendent.
  // Note that this test is very costly, up to O(tree_size).
  // Since tree_size can grow O(N), this check is O(N) in worst case,
  // where N is total number of data nodes.
  // ================================================================
  static bool isDescendent(auto myNewParent, auto sptr_mynode) {
    bool isDescendent = false;
    auto parentsParent = myNewParent->parent;
    while (auto sptrPP = parentsParent.lock()) {
      if (sptrPP == sptr_mynode) {
        isDescendent = true;
        break;
      }
      parentsParent = sptrPP->parent;
    }
    return isDescendent;
  }

  // remove me from the 'owns' list of myNewParent owner
  static bool removeFromOwnsList(auto sptrOwner, auto sptrOwned) {
    assert(sptrOwner->owns.size() > 0);
    bool removed = false;
    for (unsigned i = 0; i < sptrOwner->owns.size(); i++)
      if (sptrOwner->owns[i].lock().get() == sptrOwned.get()) {
        sptrOwner->owns.erase(sptrOwner->owns.begin() + i);
        removed = true;
        break;
      }
    return removed;
  }

  // remove other from my 'owned_by' list of sptr_myWeakOwner owner
  static bool removeFromOwnedByList(auto sptr_myWeakOwner, auto sptr_mynode) {
    assert(sptr_mynode->owned_by.size() > 0);
    bool removed = false;
    for (unsigned i = 0; i < sptr_mynode->owned_by.size(); i++)
      if (sptr_mynode->owned_by[i].lock().get() == sptr_myWeakOwner.get()) {
        sptr_mynode->owned_by.erase(sptr_mynode->owned_by.begin() + i);
        removed = true;
        break;
      }
    return removed;
  }
};

}  // namespace cycles

#endif  // CYCLES_TNODE_HPP_ // NOLINT
