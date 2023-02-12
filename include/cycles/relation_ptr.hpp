// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_RELATION_PTR_HPP_  // NOLINT
#define CYCLES_RELATION_PTR_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <string>  // ONLY FOR getType HELPER??
#include <utility>
#include <vector>

//
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/DynowForestV1.hpp>
#include <cycles/detail/v1/TArrowV1.hpp>
#include <cycles/detail/v1/TreeV1.hpp>
#include <cycles/relation_pool.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// =================================
// relation_ptr using DynowForestV1
// =================================
// smart pointer suitable for cycles
// memory is self-managed
//----------------------------------

namespace cycles {

// NOLINTNEXTLINE
using namespace detail;

template <typename T, class DOF = DynowForestV1>
// NOLINTNEXTLINE
class relation_ptr {
  using X = TNodeData;

#ifdef CYCLES_TEST
 public:  // NOLINT
#endif

#ifdef WEAK_POOL_PTR
  wptr<DOF> ctx;
#else
  sptr<DOF> ctx;
#endif
  //
  TArrowV1<X> arrow;

 public:
  using pool_type = DOF;

  /*
    void setDebug(bool b) {
      debug_flag_ptr = b;
      arrow.setDebug(b);
    }
    */

 public:
#ifdef WEAK_POOL_PTR
  sptr<DOF> get_ctx() const { return ctx.lock(); }
#else
  sptr<DOF> get_ctx() const { return ctx; }
#endif

  // ======= C[-1] constructor (weak self) ======
  // struct weak_self {};
  //
  // C[-1] constructor for Strong Self links is a bad idea!
  // ====> Tested Strategy (not promising at all!):
  // 1. will store T* t on new TNode
  // 2. will connect TNode STRONGLY pointing to ITSELF
  // 3. will NOT create a Tree... it's all alone!
  //
  // relation_ptr(wptr<DOF> _ctx, T* t, weak_self flag)
  //     : ctx{_ctx} {...}

  // ======= C0 nullptr =======
  relation_ptr() { setup_c0(); }

  // ======= C0' nullptr (allow nullptr implicit conversion) ===============
  // NOLINTNEXTLINE
  relation_ptr(std::nullptr_t t) { setup_c0(); }

  // implementation for constructor C0 and C0'
  void setup_c0() {
    this->arrow = TArrowV1<X>{};
    assert(arrow.is_null());
  }

  // ======= C1' - spointer constructor =======
  // 1. will store T* t owned by new local shared_ptr 'ref'
  // 2. will create a new TNode , also carrying shared_ptr 'ref'
  // 3. will create a new Tree and point
  relation_ptr(T* t, const relation_pool<DOF>& _pool)
      : ctx{_pool.getContext()} {
    setup_c1(t);
  }

  // ======= C1 - spointer constructor =======
  // 1. will store T* t owned by new local shared_ptr 'ref'
  // 2. will create a new TNode , also carrying shared_ptr 'ref'
  // 3. will create a new Tree and point
  relation_ptr(T* t, wptr<DOF> _ctx) : ctx{_ctx} { setup_c1(t); }

  // implementation for constructor C1 and C1'
  void setup_c1(T* t) {
    // if no context or null pointer, this is null arrow
    if ((!t) || (!get_ctx())) {
      this->arrow = TArrowV1<X>{};
      assert(arrow.is_null());
      return;
    }
    // keep local until passed to forest
    sptr<TNodeData> ref = TNodeData::make_sptr<T>(t);
    // sanity check on 'make_sptr'
    assert(ref);
    // using op1: we only store weak reference here
    this->arrow = get_ctx()->op1_addNodeToNewTree(ref);
    // sanity check on 'op1'
    assert(!this->arrow.is_owned_by_node);
    assert(arrow.is_root());
  }

  // C2 CONSTRUCTOR - EQUIVALENT TO C1+C4
  relation_ptr(T* t, const relation_ptr<T>& owner) : ctx{owner.ctx} {
    // if no context or null pointer, this is null arrow
    if ((!t) || (!get_ctx()) || owner.arrow.is_null()) {
      this->arrow = TArrowV1<X>{};
      assert(arrow.is_null());
      return;
    }
    // KEEP LOCAL
    sptr<TNodeData> ref = TNodeData::make_sptr<T>(t);
    // sanity check on 'make_sptr'
    assert(ref);
    // invoke op2
    this->arrow =
        get_ctx()->op2_addChildStrong(owner.arrow.remote_node.lock(), ref);
    // sanity check
    assert(this->arrow.is_owned());
  }

 private:
  // ======= C3 copy constructor (DELETED) =======
  relation_ptr(const relation_ptr<T>& copy) = delete;

 public:
  // ======= C4 copy constructor WITH owner =======
  relation_ptr(const relation_ptr<T>& copy, const relation_ptr<T>& owner)
      : ctx{copy.ctx} {
    // if no context or null pointer, this is null arrow
    if (!get_ctx() || copy.arrow.is_null() || owner.arrow.is_null()) {
      this->arrow = TArrowV1<X>{};
      assert(arrow.is_null());
      return;
    }

    // Runtime Check: same ctx for both pointers
    assert(get_ctx().get() == owner.get_ctx().get());
    //
    assert(this != &copy);   // IMPOSSIBLE
    assert(this != &owner);  // IMPOSSIBLE
    // ==========================================================
    // both nodes exist already (copy node and owner node)
    // register WEAK ownership in tree using 'op3_weakSetOwnedBy'
    //
    this->arrow = get_ctx()->op3_weakSetOwnedBy(copy.arrow.remote_node.lock(),
                                                owner.arrow.remote_node.lock());
    // sanity check
    assert(this->arrow.is_owned());
    // sanity check: OWNER MUST EXIST... AT LEAST FOR NOW!
    assert(this->arrow.owned_by_node.lock());
  }

  // friend helper for constructor M1
  template <typename U, typename DOF2>
  friend class relation_ptr;

  // ======= M1 move constructor =======
  // IMPORTANT 1: allow conversion from any type U, such that U* converts to T*
  // IMPORTANT 2: do not restrict it over implicit conversions
  template <class U, class = typename std::enable_if<
                         std::is_convertible<U*, T*>::value, void>::type>
  relation_ptr(relation_ptr<U, DOF>&& corpse) noexcept  // NOLINT
      : ctx{std::move(corpse.ctx)}, arrow{std::move(corpse.arrow)} {}

 public:
  // ========== destructor ==========
  ~relation_ptr() { destroy(); }

  void reset() { destroy(); }

 private:
  void destroy() {
    // sanity check
    assert(this->arrow.is_null() || this->arrow.is_root() ||
           this->arrow.is_owned());
    //
    if (this->arrow.is_null()) {
      // nothing to do!
      // arrow will be fully cleared in next step
    } else {
      assert(this->arrow.is_root() || this->arrow.is_owned());
      this->get_ctx()->op4_remove(this->arrow);
      // end-if is_root || is_owned
    }

    // CLEAR (even if it's null already...)
    this->arrow = TArrowV1<X>{};
    assert(this->arrow.is_null());
//
#ifdef WEAK_POOL_PTR
    // nothing to do
#else
    // reset shared structure for context
    this->ctx = nullptr;
#endif
  }

 private:
  // no copy assignment
  relation_ptr& operator=(const relation_ptr& other) = delete;

 public:
  // move assignment
  relation_ptr& operator=(relation_ptr&& corpse) noexcept {
    destroy();
    this->ctx = std::move(corpse.ctx);
    this->arrow = std::move(corpse.arrow);
    return *this;
  }

  // =============== GET OPERATIONS ===============

  // get_owned: returns a relation_ptr pointing to owner's data and setup
  // ownership relationship to 'owner'
  //
  // example: b = a.get_owned(c);
  // b is a copy of a's pointer, and relationship c->b is created
  // (so relation "c owns a", c->a, is kept on object b)
  //
  auto get_owned(const relation_ptr<T>& owner) const {
    // C4 constructor
    // NOTE: this cannot be nullptr
    assert(!this->arrow.is_null());
    // NOTE: owner cannot be nullptr
    assert(!owner.arrow.is_null());
    // int sz_owns = owner.arrow.remote_node.lock()->owns.size();
    auto r = relation_ptr<T>(*this, owner);
    // assert(sz_owns + 1 == owner.arrow.remote_node.lock()->owns.size());
    //
    return r;
  }

  // create self-owned reference (similar to "weak reference")
  auto get_self_owned() const {
    auto self_ptr = this->get_owned(*this);
    return self_ptr;
  }

  // NOT implemented yet
  auto get_unowned() {
    // cannot get pointer from null
    if (this->arrow.is_null()) {
      // return null
      return relation_ptr<T>{};
    }
    // cannot currently make copies of unowned
    if (this->arrow.is_root()) {
      // return null
      return relation_ptr<T>{};
    }
    // this must be owned
    assert(this->arrow.is_owned());
    std::cout << "ERROR: must implement unowned from owned logic!" << std::endl;
    assert(false);
    return relation_ptr<T>{};
  }

  bool operator==(const relation_ptr<T>& other) const {
    // context and pointers should be the same
    return (get_ctx() == other.get_ctx()) && (get() == other.get());
  }

 private:
  // same as operator bool()
  bool has_get() const noexcept {
    // NOLINTNEXTLINE
    return (bool)get();
  }

 public:
  // this typically replaces 'has_get'
  explicit operator bool() const noexcept { return has_get(); }

  // returns shared pointer to data
  sptr<T> get_shared() const {
    // TODO: use arrow.get_data_shared()
    auto sremote_node = this->arrow.remote_node.lock();
    if (!sremote_node) {
      return nullptr;
    } else {
      // NOLINTNEXTLINE
      return sptr<T>{sremote_node->value, (T*)(sremote_node->value->p)};
    }
  }

  // returns raw pointer to data
  T* get() const {
    auto mysptr = get_shared();
    if (!mysptr)
      return nullptr;
    else
      return mysptr.get();
  }

  // typical navigation operator
  T* operator->() const { return get(); }

  // const operator* (SFINAE disables for T=void)
  template <typename U = T>
  typename std::enable_if<!std::is_same<U, void>::value, const U&>::type  //
  operator*() const {
    T* ptr = get();
    assert(ptr);
    return *ptr;
  }

  // operator* (SFINAE disables for T=void)
  template <typename U = T>
  typename std::enable_if<!std::is_same<U, void>::value, U&>::type  //
  operator*() {
    T* ptr = get();
    assert(ptr);
    return *ptr;
  }

 public:
  // TODO: improve with multiple 'make' methods...
  // IDEA: 'make', 'make_owned', 'make_unowned' (same as 'make'), etc
  template <class... Args>
  static relation_ptr<T> make(sptr<DynowForestV1> ctx, Args&&... args) {
    // NOLINTNEXTLINE
    auto* t = new T(std::forward<Args>(args)...);
    return relation_ptr<T>{t, ctx};
  }
};

}  // namespace cycles

#endif  // CYCLES_RELATION_PTR_HPP_ // NOLINT
