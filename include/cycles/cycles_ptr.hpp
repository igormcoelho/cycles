// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_cycles_ptr_HPP_  // NOLINT
#define CYCLES_cycles_ptr_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
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
// NOLINTNEXTLINE
class cycles_ptr {
 private:
  wptr<cycles_ctx<T>> ctx;
  //
  // TODO: should be an element/vertex on tree
  // sptr<T> ref; // TODO: remove this direct ref!
  //
  wptr<TNode<sptr<T>>> remote_node;
  bool debug_flag;

 public:
  auto get_ctx() const { return ctx; }

  bool debug() const { return debug_flag; }

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
    this->debug_flag = get_ctx().lock()->debug;
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
    if (debug())
      std::cout << "=> C1 pointer constructor: Registering this in new Tree on "
                   "Context!"
                << std::endl;
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

  // private:
  // ALLOW THIS, FOR NOW!
  //
  // ======= C2 pointer constructor WITH owner =======
  // 1. will store T* t owned by new local shared_ptr 'ref'
  // 2. will create a new TNode , also carrying shared_ptr 'ref'
  // 3. will create a new Tree and point
  cycles_ptr(wptr<cycles_ctx<T>> ctx, T* t, cycles_ptr<T>& owner)
      : ctx{ctx}  //, ref { t }
                  //, remote_node { !t ? nullptr : sptr<TNode<sptr<T>>>(new
                  // TNode<sptr<T>> { this->ref }) }
  {
    sptr<T> ref{t};
    // WE NEED TO HOLD SPTR locally, UNTIL we store it in definitive sptr...
    // this 'remote_node' is weak!
    auto sptr_remote_node =
        !t ? nullptr : sptr<TNode<sptr<T>>>(new TNode<sptr<T>>{ref});
    // we only hold weak reference here
    this->remote_node = sptr_remote_node;
    this->debug_flag = get_ctx().lock()->debug;
    //
    if (debug()) {
      std::cout
          << "C2 pointer constructor: creating NEW OWNED cycles_ptr (this_new="
          << this << " to t*=" << t << ") "
          << " owner='" << owner.get() << "' ";
      if (ref)
        std::cout << "with ref -> " << *ref << std::endl;
      else
        std::cout << "with ref -> nullptr" << std::endl;
    }
    //
    if (!ref) {
      // THIS IS A STRANGE SITUATION... CHECK IF IT'S REALLY USEFUL!
      assert(false);
      return;  // SHOULD NOT CREATE A NEW TREE OR A NEW RELATION
    }
    //
    if (debug()) {
      std::cout
          << "=> C2 pointer constructor: Registering this in EXISTING Tree "
             "context!"
          << std::endl;
    }
    //
    // auto node_new = sptr<TNode<sptr<T>>>(new TNode<sptr<T>> { ref });
    // remote_node = node_new;
    assert(sptr_remote_node);
    //
    // IMPORTANT! TREE OF OWNER MUST EXIST... BUT...
    // ... WE CANNOT LOCATE IT ANYMORE (removed field 'tree_root')
    //
    // so, just check context and...
    assert(this->ctx.lock() == owner.ctx.lock());
    // ... and STRONG point from owner node to new node
    owner.remote_node.lock()->add_child_strong(sptr_remote_node);
    if (debug()) ctx.lock()->print();
    //
  }

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
      : ctx{corpse.ctx}, remote_node{std::move(corpse.remote_node)} {
    corpse.remote_node.reset();
    this->debug_flag = get_ctx().lock()->debug;
  }

 public:
  // ======= C4 copy constructor WITH owner =======
  // copy constructor (still good for vector... must be the meaning of a "copy")
  // proposed operation is:
  // cptr1 = cycles_ptr<T>(cptr0, cptr2); // (cptr0 and cptr2 exists already)
  // it could also follow this logic:
  // 1. cptr1 = cycles_ptr<T>{cptr0}; // copy cptr0 into cptr1
  // 2. cptr1.set_owned_by(cptr2);   // makes cptr1 (and also cptr0) owned by
  // cptr2
  //
  cycles_ptr(const cycles_ptr<T>& copy, const cycles_ptr<T>& owner)
      : ctx{copy.ctx}, remote_node{copy.remote_node} {
    this->debug_flag = get_ctx().lock()->debug;
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
    // register ownership
    this->set_owned_by(owner);
  }

  int get_ref_use_count() const { return this->get_sptr().use_count(); }

  void destroy() {
    if (debug()) {
      std::cout << "destroy: ref_use_count=" << this->get_ref_use_count();
      if (!has_get())
        std::cout << "{NULL}";
      else
        std::cout << " {" << this->get() << "}";
      std::cout << std::endl;
    }
    // this->ref = nullptr;
    this->remote_node = wptr<TNode<sptr<T>>>();  // clear
  }

  ~cycles_ptr() {
    if (debug()) std::cout << "begin ~cycles_ptr" << std::endl;
    destroy();
    if (debug()) std::cout << "end ~cycles_ptr" << std::endl;
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
      }
      return;
    }
    //
    unsafe_set_owned_by(owner);
  }

  // check if this pointer is root (in tree/forest universe)
  bool is_root() const { return (!this->remote_node.lock()->has_parent()); }

  // the unsafe method will not check if it's already owner...
  // this strongly reduces computational cost (NOT inspecting child list)
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
    //
    // TREE OF OWNER MUST EXIST... BUT... WE CANNOT CHECK IT ANYMORE (removed
    // 'tree_root') WRONG!!!! CASE 1) IF... this is root (no parent), then
    // 'owner' can really take this (STRONGLY).. CASE 2) otherwise, just add a
    // weak link from 'owner' to this node.
    //
    // SECOND TRY!!
    //
    // CASE 1) IF... owner is root (no parent), then 'owner' can really take
    // this (STRONGLY) CASE 1.1) IF myself is also root, then my tree is
    // removed; otherwise, I'm just removed from children list. CASE 2)
    // otherwise, just add a weak link from 'owner' to this node.
    // =========== CASE 1 ===========
    // if (!owner.remote_node.lock()->parent.lock()) {
    if (owner.is_root()) {
      if (debug()) {
        std::cout << "CASE 1: owner is root of some tree! Solution: OWNER will "
                     "take it!"
                  << std::endl;
      }
      // //std::cout << "HOWEVER.... IF owner also belongs to this same tree,
      //  this should be a WEAK LINK...." << std::endl;
      //
      // STEP 1 - add strong link from OWNER to THIS
      // store weak link to old parent
      auto old_parent = this->remote_node.lock()->parent;
      //
      // update parent before calling method
      this->remote_node.lock()->parent = owner.remote_node;
      owner.remote_node.lock()->add_child_strong(this->remote_node.lock());
      //
      // =========== CHECK CASE 1.1
      //
      if (!old_parent.lock()) {
        // CASE 1.1A - I was also root! must remove my Tree!!!
        //
        if (debug()) {
          std::cout
              << "  => CASE 1.1A - I'm also root, should remove this Tree "
                 "from forest..."
              << std::endl;
        }
        //
        auto tree_it = ctx.lock()->forest.find(this->remote_node.lock());
        if (tree_it == ctx.lock()->forest.end()) {
          std::cout << "CASE 1.1 ERROR! could not find Tree!" << std::endl;
          assert(false);
        } else {
          if (debug()) {
            std::cout
                << " ~~~> CASE 1.1 OK FOUND MY TREE. How to properly clear it?"
                << std::endl;
            std::cout << "First try, just drop it, and let all nodes die "
                         "automatically"
                      << std::endl;
          }
          // auto stree = tree_it->second;
          ctx.lock()->forest.erase(tree_it);
          // clear tree
        }
      } else {
        // CASE 1.1B - I'm not root... must remove me from my old_parent
        // children list
        //
        std::cout << "  => CASE 1.1B - I'm not root, must remove me from "
                     "old_parent children list..."
                  << std::endl;
        bool b =
            old_parent.lock()->remove_child(this->remote_node.lock().get());
        assert(b);
      }
      if (debug()) std::cout << "CASE 1: finished!" << std::endl;
    } else {
      // =========== CASE 2 ===========
      if (debug()) {
        std::cout << "CASE 2: owner is already part of some tree! Solution: "
                     "OWNER will add weak link to it!"
                  << std::endl;
      }
      owner.remote_node.lock()->add_child_weak(this->remote_node);
    }

    ctx.lock()->print();
    //
  }

  // no copy assignment
  cycles_ptr& operator=(const cycles_ptr& other) = delete;

  cycles_ptr& operator=(cycles_ptr&& corpse) noexcept {
    if (debug()) std::cout << "begin operator==(&&)" << std::endl;
    destroy();
    if (debug()) std::cout << "will move assign" << std::endl;
    this->ctx = std::move(corpse.ctx);
    this->remote_node = std::move(corpse.remote_node);
    if (debug()) std::cout << "end operator==(&&)" << std::endl;

    return *this;
  }

  // =============== BASE OPERATIONS ===============

  // returns a self-copy and setup ownership relationship to 'owner'
  //
  // important: (this is an idea)
  // - if you copy pointer again (with regular copy constructor),
  // ownership relationship will be kept on ctx
  // example: b = a.copy_owned(c); // b is a copy of a, and relationship c->b is
  // created (so as "c owns a",  c->a)
  // - maybe this is a good thing, because we can keep copy constructor
  // - maybe not, but I don't imagine why at this moment...
  auto copy_owned(const cycles_ptr<T>& owner) {
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
    // do not comparing null pointers as 'true' (why?)... just feels like right
    // now. (thinking more of refs than pointers)
    // (this->has_get() && other.has_get()) &&
    // TODO: think more.
    return (this->has_get() == other.has_get()) &&
           (ctx.lock() == other.ctx.lock()) &&
           (get_ptr() && other.get_ptr());  //&& (ref == other.ref);
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