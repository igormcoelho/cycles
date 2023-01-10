// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_cycles_ptr_HPP_  // NOLINT
#define CYCLES_cycles_ptr_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <string>  // ONLY FOR getType HELPER??
#include <utility>
#include <vector>

//
#include <cycles/Tree.hpp>
#include <cycles/cycles_ctx.hpp>
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
  static bool removeFromOwnsList(auto myNewParent, auto sptr_mynode) {
    assert(myNewParent->owns.size() > 0);
    bool removed = false;
    for (unsigned i = 0; i < myNewParent->owns.size(); i++)
      if (myNewParent->owns[i].lock().get() == sptr_mynode.get()) {
        myNewParent->owns.erase(myNewParent->owns.begin() + i);
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

template <typename T>
// NOLINTNEXTLINE
class cycles_ptr {
  // TODO(igormcoelho): make private!
 public:
  //
  wptr<cycles_ctx<T>> ctx;
  //
  wptr<TNode<sptr<T>>> remote_node;
  //
  // NOTE THAT is_owned_by_node MAY BE TRUE, WHILE owned_by_node
  // BECOMES UNREACHABLE... THIS HAPPENS IF OWNER DIES BEFORE THIS POINTER.
  // THE RELATION SHOULD BE IMMUTABLE, IT MEANS THAT ONCE "OWNED", ALWAYS
  // "OWNED".
  wptr<TNode<sptr<T>>> owned_by_node;
  bool is_owned_by_node{false};
  //
  bool debug_flag_ptr{false};

 public:
  void setDebug(bool b) {
    debug_flag_ptr = b;
    auto node = remote_node.lock();
    if (node) node->debug_flag = b;
  }

 public:
  auto get_ctx() const { return ctx; }

  bool debug() const { return debug_flag_ptr; }

  // ======= C1 - spointer constructor =======
  // 1. will store T* t owned by new local shared_ptr 'ref'
  // 2. will create a new TNode , also carrying shared_ptr 'ref'
  // 3. will create a new Tree and point
  cycles_ptr(wptr<cycles_ctx<T>> ctx, T* t) : ctx{ctx} {
    sptr<T> ref{t};  // LOCAL!
    // WE NEED TO HOLD SPTR locally, UNTIL we store it in definitive sptr...
    // this 'remote_node' is weak!
    auto sptr_remote_node =
        !t ? nullptr : sptr<TNode<sptr<T>>>(new TNode<sptr<T>>{ref});
    // we only hold weak reference here
    this->remote_node = sptr_remote_node;
    //
    this->debug_flag_ptr = get_ctx().lock()->debug;
    //
    if (debug()) {
      std::cout << "C1 pointer constructor: creating NEW cycles_ptr (this_new="
                << this << " to t*=" << t << ") ";
      if (ref)
        std::cout << "with ref -> " << *ref << std::endl;
      else
        std::cout << "with ref -> nullptr" << std::endl;
    }
    //
    if (!ref) {
      return;  // SHOULD NOT CREATE A NEW TREE!
    }
    if (!(this->remote_node.lock())) {
      // STRANGE ERROR! SHOULD NEVER OCCUR!
      assert(false);
      return;
    }
    //
    if (debug()) {
      std::cout << "=> C1 pointer constructor: Registering this in new Tree on "
                   "Context!"
                << std::endl;
    }
    // auto node_new = sptr<TNode<sptr<T>>>(new TNode<sptr<T>> { ref });
    // this->remote_node = node_new;
    //
    auto stree = sptr<Tree<sptr<T>>>(new Tree<sptr<T>>{});
    if (debug()) {
      std::cout << "tree ~> ";
      stree->print();
      std::cout << "Printed Tree!" << std::endl;
    }
    // TODO: how could this fail? IMPORTANT test!
    assert(sptr_remote_node);
    // STRONG storage of remote node pointer
    stree->set_root(sptr_remote_node);
    ctx.lock()->forest[sptr_remote_node] = stree;
    if (debug()) ctx.lock()->print();
    // stree.root = sptr<TNode<T>>(new TNode<T>(val, stree));
    // forest[ref] = stree;
    if (debug())
      std::cout << " -> finished C1 pointer constructor" << std::endl;
  }

  // NO C2 ANYMORE - CONSTRUCTOR REMOVED!
  // cycles_ptr(wptr<cycles_ctx<T>> ctx, T* t, cycles_ptr<T>& owner) = 0;

 private:
  //
  // NO COPY CONSTRUCTOR
  //
  // ======= C3 copy constructor =======
  cycles_ptr(const cycles_ptr<T>& copy) = delete;

 public:
  // ======= M1 move constructor =======
  // simply move smart pointer to all elements: ctx, ref and remote_node
  cycles_ptr(cycles_ptr<T>&& corpse) noexcept
      : ctx{corpse.ctx},
        remote_node{std::move(corpse.remote_node)},
        owned_by_node{std::move(corpse.owned_by_node)},
        is_owned_by_node{std::move(corpse.is_owned_by_node)} {
    corpse.remote_node.reset();
    corpse.owned_by_node.reset();
    corpse.is_owned_by_node = false;
    this->debug_flag_ptr = get_ctx().lock()->debug;
  }

 public:
  // ======= C4 copy constructor WITH owner =======
  // copy constructor (still good for vector... must be the meaning of a
  // "copy") proposed operation is: cptr1 = cycles_ptr<T>(cptr0, cptr2); //
  // (cptr0 and cptr2 exists already) it could also follow this logic:
  // 1. cptr1 = cycles_ptr<T>{cptr0}; // copy cptr0 into cptr1
  // 2. cptr1.set_owned_by(cptr2);   // makes cptr1 (and also cptr0) owned by
  // cptr2
  //
  cycles_ptr(const cycles_ptr<T>& copy, const cycles_ptr<T>& owner)
      : ctx{copy.ctx}, remote_node{copy.remote_node} {
    this->debug_flag_ptr = get_ctx().lock()->debug;
    if (debug()) std::cout << "c4 constructor for copy_owned" << std::endl;
    if (this == &copy) {
      // TODO: self assignment breaks nothing here, in this case...
      // ... but let's avoid it, for now
      assert(false);
    }
    if (this == &owner) {
      // TODO: this is strange...
      // ... so let's avoid it, for now
      assert(false);
    }
    //
    // register ownership in tree
    this->set_owned_by(owner);
    // remember ownership (for future deletion?)
    this->owned_by_node = owner.remote_node;
    this->is_owned_by_node = true;
    // OWNER MUST EXIST... AT LEAST NOW!
    assert(this->owned_by_node.lock());
    if (debug())
      std::cout << "finish c4: stored owner in owned_by_node" << std::endl;
  }

  int get_ref_use_count() const { return this->get_sptr().use_count(); }

  void destroy_tree(sptr<TNode<sptr<T>>> sptr_mynode) {
    if (debug()) std::cout << "destroy: will destroy my tree." << std::endl;
    // find my tree
    auto tree_it = ctx.lock()->forest.find(sptr_mynode);
    if (tree_it == ctx.lock()->forest.end()) {
      // ????
      std::cout << "ERROR! COULD NOT FIND MY TREE!" << std::endl;
      assert(false);
    } else {
      if (debug()) {
        std::cout << " ~~~> OK FOUND MY TREE. Delete it." << std::endl;
      }
      // clear tree
      ctx.lock()->forest.erase(tree_it);

      if (debug()) {
        std::cout << " ~~~> OK DELETED MY TREE." << std::endl;
      }
    }
  }

  void destroy() {
    if (debug()) std::cout << "destroy: BEGIN" << std::endl;
    // TODO(igormcoelho): how to manage "delegated sptr" pointers here?
    // Maybe just consider some "unique_ptr forest" for now?
    //
    if (debug()) {
      std::cout << "destroy: ref_use_count=" << this->get_ref_use_count();
      if (!has_get())
        std::cout << " get(): NULL";
      else
        std::cout << " get(): (" << this->get() << ")";
      std::cout << std::endl;
    }
    // BEGIN complex logic
    // where is nullptr here? TODO: fix
    assert(is_nullptr() || is_root() || is_owned());
    //
    if (is_nullptr()) {
      if (debug()) std::cout << "destroy: is_nullptr()" << std::endl;
      return;  // nothing to destroy
               // (MUST KEEP else below, otherwise it may break(?))
    } else {
      //
      bool isRoot = is_root();
      bool isOwned = is_owned();
      //
      assert(isRoot || isOwned);
      //
      if (debug()) std::cout << "destroy: is_root() || is_owned()" << std::endl;
      // must find someone to strongly own me, otherwise node may die!
      // First:  find someone in my 'owned_by' list
      bool will_die = false;
      auto sptr_mynode = this->remote_node.lock();
      auto owner_node = this->owned_by_node.lock();
      //
      // check situation of node (if dying or not)
      //
      if (isRoot) {
        // this node is root in tree!
        if (debug()) std::cout << "DEBUG: I AM ROOT! I WILL DIE!" << std::endl;
        will_die = true;
      }  // is_root
      //
      if (isOwned) {
        // check if owner still exists
        if (debug()) std::cout << "DEBUG: I AM OWNED!" << std::endl;
        if (!owner_node) {
          // SHOULD THIS BEHAVE AS is_nullptr?
          if (debug()) std::cout << "WARNING: avestruz!" << std::endl;
          will_die = false;
        } else {
          // CHECK IF OWNER IS MY PARENT...
          if (owner_node == sptr_mynode->parent.lock()) {
            std::cout << "DEBUG: OWNER IS MY PARENT! I WILL DIE!" << std::endl;
            will_die = true;
          } else {
            std::cout << "DEBUG: OWNER IS NOT MY PARENT! I WILL NOT DIE!"
                      << std::endl;
            // my node will stay alive since my parent still holds me strong
            will_die = false;
            // remove my weak link from owner
            bool r0 = TNodeHelper<sptr<T>>::removeFromOwnsList(owner_node,
                                                               sptr_mynode);
            assert(r0);
            // remove owner from my weak link list
            bool r1 = TNodeHelper<sptr<T>>::removeFromOwnedByList(owner_node,
                                                                  sptr_mynode);
            assert(r1);
          }
        }
      }  // is_owned
      if (!will_die) {
        if (debug())
          std::cout << "DEBUG: WILL NOT DIE. FORCE CLEAR!" << std::endl;
        // FORCE CLEAR
        this->remote_node = wptr<TNode<sptr<T>>>();    // clear
        this->owned_by_node = wptr<TNode<sptr<T>>>();  // clear
        this->is_owned_by_node = false;
        return;
      }
      assert(will_die);
      if (debug())
        std::cout
            << "DEBUG: FIND OWNED_BY. NODE WILL DIE IF NOT FIND REPLACEMENT!"
            << std::endl;
      // find new owner, otherwise will die
      for (unsigned k = 0; k < sptr_mynode->owned_by.size(); k++) {
        auto myNewParent = sptr_mynode->owned_by[k].lock();
        if (!myNewParent) {
          std::cout << "ERROR! no new parent! how??" << std::endl;
        }
        // new parent must exist
        assert(myNewParent);
        if (debug()) {
          std::cout
              << "Found new parent to own me (will check if not on subtree): "
              << myNewParent->value_to_string() << std::endl;
        }
        // NOTE: costly O(tree_size)=O(N) test in worst case for 'isDescendent'
        bool _isDescendent =
            TNodeHelper<sptr<T>>::isDescendent(myNewParent, sptr_mynode);
        //
        if (debug())
          std::cout << "DEBUG: isDescendent=" << _isDescendent << " k=" << k
                    << std::endl;
        if (_isDescendent) {
          std::cout << "WARNING: owned_by is already my descendent! Discard. "
                    << "Will try next k!"
                    << "k=" << k << std::endl;
          // k++
          continue;
        }
        // found some good k!
        // assume this will not die (for this 'k'). FOUND some good owner!
        will_die = false;
        if (debug()) {
          std::cout << "Found new VALID parent to own me: "
                    << myNewParent->value_to_string() << std::endl;
        }
        // COSTLY. Remove me from the 'owns' list of my owner
        bool removed =
            TNodeHelper<sptr<T>>::removeFromOwnsList(myNewParent, sptr_mynode);
        assert(removed);
        // delete myself from owned_by (now I'm strong child)
        sptr_mynode->owned_by.erase(sptr_mynode->owned_by.begin() + k);
        // add myself as myNewParent child
        sptr_mynode->parent = myNewParent;
        myNewParent->add_child_strong(sptr_mynode);
        // will_die should be False, at this point
        if (!will_die) break;
      }  // end for k
      //
      if (will_die) {
        if (debug())
          std::cout << "destroy: will_die is TRUE. MOVE TO GARBAGE????"
                    << std::endl;
        // MOVE TO GARBAGE?
        // assert(false);
      }
      // CLEAR!
      if (debug())
        std::cout << "CLEAR STEP: will_die = " << will_die << std::endl;

      // prepare final destruction
      if (debug())
        std::cout << "DEBUG: will check situation: either is_root or is_owned"
                  << std::endl;
      if (isRoot) {
        if (debug())
          std::cout << "DEBUG: is_root. destroy_tree(...)" << std::endl;
        destroy_tree(sptr_mynode);
      }
      if (isOwned) {
        if (debug())
          std::cout << "DEBUG: is_owned. owner_node->remove_child(...)"
                    << std::endl;
        bool r = owner_node->remove_child(sptr_mynode.get());
        assert(r);
      }
      // last holding reference to node is sptr_mynode
      if (debug())
        std::cout << "destroy: last call to 'sptr_mynode'" << std::endl;
      if (debug()) sptr_mynode->debug_flag = true;
      // manual/explicit deletion
      sptr_mynode = nullptr;
      if (debug())
        std::cout << "destroy: destroyed 'sptr_mynode' (AND WHOLE TREE "
                     "BELOW... PROBABLY NOT GOOD!)"
                  << std::endl;
      //
      // end-if is_root || is_owned
    }

    if (debug()) std::cout << "destroy: last cleanups" << std::endl;
    //
    // this->ref = nullptr;
    this->remote_node = wptr<TNode<sptr<T>>>();    // clear
    this->owned_by_node = wptr<TNode<sptr<T>>>();  // clear
    this->is_owned_by_node = false;
    if (debug()) std::cout << "destroy: END" << std::endl;
  }

  void reset() {
    if (debug()) std::cout << "cycles_ptr::reset() BEGIN" << std::endl;
    destroy();
    if (debug()) std::cout << "cycles_ptr::reset() END" << std::endl;
  }

  // HELPER: is_nullptr, is_root, is_owned
  std::string getType() {
    if (is_nullptr())
      return "is_nullptr";
    else if (is_root())
      return "is_root";
    else
      return "is_owned";
  }

  ~cycles_ptr() {
    if (debug())
      std::cout << "begin ~cycles_ptr() getType=" << getType() << std::endl;
    destroy();
    if (debug()) std::cout << "end ~cycles_ptr()" << std::endl;
  }

  // =============================
  // this will be owned by 'owner'
  // =============================

  void set_owned_by(const cycles_ptr<T>& owner) {
    if (this == &owner) {
      // TODO: this is strange...
      // ... so let's avoid it, for now
      assert(false);
    }
    // guarantee that context is the same
    assert(this->ctx.lock() == owner.ctx.lock());
    //
    // std::cout << "TODO: check if owner ALREADY has a weak or strong link to
    // this node... don't know what could happen!" << std::endl;
    if (owner.remote_node.lock()->has_child(this->remote_node)) {
      if (debug()) {
        std::cout << "set_owned_by WARNING! prevented double linking child... "
                     "ALREADY owner!"
                  << std::endl;
        std::cout << "TODO: maybe need to allow double linking here..."
                  << std::endl;
      }
      return;
    }
    //
    unsafe_set_owned_by(owner);
  }

  // ========== TWO FUNDAMENTAL PROPERTIES ===========
  // A) is_nullptr
  // B) is_root
  // C) is_owned
  // Node should respect: is_nullptr() || is_root() || is_owned()
  //

  // check if this pointer is nullptr
  bool is_nullptr() const { return (!this->remote_node.lock()); }

  // check if this pointer is root (in tree/forest universe)
  bool is_root() const {
    // IMPORTANT! DO NOT REMOVE is_owned() check from here!!
    if (is_nullptr() || is_owned())
      return false;
    else
      return (!this->remote_node.lock()->has_parent());
  }

  // check if this pointer is already owned by someone
  bool is_owned() const {
    bool b1 = is_owned_by_node;
    // NOLINTNEXTLINE
    bool b2 = (bool)(this->owned_by_node.lock());
    if (b1 && !b2) {
      if (debug())
        std::cout << "cycle_ptr is_owned() WARNING: is_owned_by_node but owner "
                     "does not exist!"
                  << std::endl;
    }
    return b1;
  }

  int count_owned_by() const {
    auto node_ptr = this->remote_node.lock();
    assert(node_ptr);
    return node_ptr->owned_by.size();
  }

  auto getOwnedBy(int idx) const {
    auto node_ptr = this->remote_node.lock();
    assert(node_ptr);
    return node_ptr->owned_by[idx].lock();
  }
  // =========================

  // new logic here
  void unsafe_set_owned_by(const cycles_ptr<T>& owner) {
    //
    if (debug()) {
      std::cout << std::endl << "cycles_ptr:: unsafe_set_owned_by" << std::endl;
      std::cout << "TODO: Must register relation of:" << std::endl;
      std::cout << "\tthis=" << this
                << " this->remote_node=" << this->remote_node.lock() << ") '"
                << (this->get()) << "' owned_by:" << std::endl;
      std::cout << "\t&owner=" << &owner << " '" << (owner.get())
                << "'  owner.is_root()=" << owner.is_root() << std::endl;
    }
    // I think previous DEPRECATED logic is messed up... trying again!
    //
    // Properties:
    // X0: Owner and I are different! (no self arc here)
    // X1: this never creates strong links (only destruction/deletion does
    // that)
    //     Note that the Strong maintainance of the forest is kept by parent
    //     and child nodes, thus all extra connections are weak links.
    //
    // It seems that only one case must exist here
    //   => SOLUTION: Owner will add a weak link to Me.

    // TODO(igormcoelho): Maybe... check if it's not yet child?
    // OLD:
    // this->remote_node.lock()->add_weak_link_owned(owner.remote_node);
    // NEW:
    TNode<sptr<T>>::add_weak_link_owned(this->remote_node.lock(),
                                        owner.remote_node.lock());
    //
    if (debug())
      std::cout << "owner |children|="
                << this->remote_node.lock()->children.size() << std::endl;
    //
    if (debug()) ctx.lock()->print();
    //
  }

  // no copy assignment
  cycles_ptr& operator=(const cycles_ptr& other) = delete;

  cycles_ptr& operator=(cycles_ptr&& corpse) noexcept {
    if (debug()) std::cout << "begin operator==(&&)" << std::endl;
    destroy();
    if (debug()) std::cout << "will move assign" << std::endl;
    this->ctx = std::move(corpse.ctx);
    this->debug_flag_ptr = corpse.debug_flag_ptr;
    this->remote_node = std::move(corpse.remote_node);
    this->owned_by_node = std::move(corpse.owned_by_node);
    this->is_owned_by_node = std::move(corpse.is_owned_by_node);
    if (debug()) std::cout << "end operator==(&&)" << std::endl;

    return *this;
  }

  // =============== BASE OPERATIONS ===============

  // returns a self-copy and setup ownership relationship to 'owner'
  //
  // important: (this is an idea)
  // - if you copy pointer again (with regular copy constructor),
  // ownership relationship will be kept on ctx
  // example: b = a.copy_owned(c); // b is a copy of a, and relationship c->b
  // is created (so as "c owns a",  c->a)
  // - maybe this is a good thing, because we can keep copy constructor
  // - maybe not, but I don't imagine why at this moment...
  auto copy_owned(const cycles_ptr<T>& owner) {
    // C4 constructor
    return cycles_ptr<T>(*this, owner);
  }

  // maybe we need an 'owns' method, that does the opposite
  // example: a.owns(b);  will create relationship a->b
  void owns(const cycles_ptr<T>& owned) { assert(false); }

  // maybe we need an 'removed_owned' method
  // MAYBE NOT! maybe this is just a ctx thing, or maybe we just let variables
  // expire to handle this... I don't really know. example: a.remove_owned(b);
  // will remove relationship a->b (if it exists, it returns true) what to do
  // with opposite relationship? do we let ctx handle all this?
  bool remove_owned(const cycles_ptr<T>& owned) { assert(false); }

  auto get_ctx() -> wptr<cycles_ctx<T>> { return ctx; }

  bool operator==(const cycles_ptr<T>& other) const {
    // do not comparing null pointers as 'true' (why?)... just feels like
    // right now. (thinking more of refs than pointers) (this->has_get() &&
    // other.has_get()) &&
    // TODO: think more.
    return (this->has_get() == other.has_get()) &&
           (ctx.lock() == other.ctx.lock()) &&
           (get_ptr() == other.get_ptr());  //&& (ref == other.ref);
  }

  bool has_get() const {
    // NOLINTNEXTLINE
    return (bool)get_ptr();
  }

  sptr<T> get_sptr() const {
    auto sremote_node = this->remote_node.lock();
    if (!sremote_node) return nullptr;
    return sremote_node->value;
  }

  T* get_ptr() const { return get_sptr().get(); }

  T& get() { return *get_ptr(); }

  const T& get() const { return *get_ptr(); }

  // TODO(igormcoelho): avoid this! but ... WHY?
  T* operator->() const { return get_ptr(); }
};

}  // namespace cycles

#endif  // CYCLES_cycles_ptr_HPP_ // NOLINT
