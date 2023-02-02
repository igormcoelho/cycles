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
#include <cycles/detail/forest_ctx.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// =======================================
// relation_pool
// =======================================
// hides implementation-specific ctx type
//----------------------------------------

namespace cycles {

// NOLINTNEXTLINE
using namespace detail;

// NOLINTNEXTLINE
class relation_pool {
 public:
  using pool_type = forest_ctx;

 private:
  // TODO(igormcoelho): ensure ctx behaves like "unique_ptr"? or allow this to
  // live as long as dependent relation_ptr exists?
  // ==== Implementation using forest_ctx ====
  sptr<forest_ctx> ctx;

 public:
  // default constructor
  relation_pool() : ctx{new forest_ctx{}} {}

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
    // clear context
    ctx = nullptr;
    // start again
    ctx = sptr<forest_ctx>{new forest_ctx{}};
  }
};

}  // namespace cycles

#endif  // CYCLES_RELATION_POOL_HPP_ // NOLINT
