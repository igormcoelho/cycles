// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_RELATION_POOL_HPP_  // NOLINT
#define CYCLES_RELATION_POOL_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <utility>
#include <vector>

//
#include <cycles/detail/v1/DynowForestV1.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// =======================================
// relation_pool
// =======================================
// hides implementation-specific DOF type
//----------------------------------------

namespace cycles {

// NOLINTNEXTLINE
using namespace detail;

// note that relation_pool is always included before relation_ptr.
// this is good!
// forward declaration of 'relation_ptr' for 'make' widget
template <class T, class DOF>
class relation_ptr;

// DOF = Dynamic Ownership Forest
// relation_pool is templated for test only...
// it could simply adopt the "best" DOF implementation.

template <class DOF = DynowForestV1>
class relation_pool {
 public:
  using pool_type = DOF;

 private:
  // TODO(igormcoelho): ensure ctx behaves like "unique_ptr"? or allow this to
  // live as long as dependent relation_ptr exists?
  // ==== Implementation using DynowForestV1 ====
  sptr<DynowForestV1> ctx;

 public:
  // default constructor
  relation_pool() : ctx{new DynowForestV1{}} {}

  // move only
  relation_pool(relation_pool&& corpse) noexcept : ctx{std::move(corpse.ctx)} {
    corpse.ctx = nullptr;
  }

  // move only
  relation_pool& operator=(relation_pool&& corpse) noexcept {
    this->ctx = std::move(corpse.ctx);
    corpse.ctx = nullptr;
    return *this;
  }

  // copy disabled
  relation_pool(const relation_pool& other) = delete;

  // copy disabled
  relation_pool& operator=(const relation_pool& other) = delete;

  ~relation_pool() {
    // std::cout << "~relation_pool: ctx=" << ctx << std::endl;
    clear();
  }

  void setAutoCollect(bool b) { ctx->setAutoCollect(b); }

  void setDebug(bool b) { ctx->setDebug(b); }

  // internal structure... TODO(igormcoelho): provide this as wptr or sptr?
  auto getContext() const { return ctx; }

  void clear() {
    // force destruction (beware: ctx could have been moved)
    if (ctx) ctx->destroyAll();
    // clear context
    ctx = nullptr;
    // start again
    ctx = sptr<DynowForestV1>{new DynowForestV1{}};
  }

  // DOF comes from this pool... T comes explicitly
  template <class T, class... Args>
  relation_ptr<T, DOF> make(Args&&... args);
};

}  // namespace cycles

#endif  // CYCLES_RELATION_POOL_HPP_ // NOLINT
