// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_DETAIL_TARROWV1_HPP_
#define CYCLES_DETAIL_TARROWV1_HPP_

// C++
#include <iostream>
#include <utility>
#include <vector>
// C++ HELPER ONLY
#include <memory>   // JUST FOR HELPER is_shared_ptr??
#include <sstream>  // just for value_to_string ??
#include <string>
//
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TNodeV1.hpp>

using std::ostream, std::vector;  // NOLINT

// ============================
//   memory-managed Tree Node
// ============================
// Tree Node
// all memory is self-managed

namespace cycles {

namespace detail {

// default is now type-erased T
template <typename T = TNodeData>
class TArrowV1 {
  using X = T;

  bool debug_flag_arrow{false};

 public:
  // TArrowV1() {}
  wptr<TNode<X>> remote_node;
  //
  // NOTE THAT is_owned_by_node MAY BE TRUE, WHILE owned_by_node
  // BECOMES UNREACHABLE... THIS HAPPENS IF OWNER DIES BEFORE THIS POINTER.
  // THE RELATION SHOULD BE IMMUTABLE, IT MEANS THAT ONCE "OWNED", ALWAYS
  // "OWNED".
  wptr<TNode<X>> owned_by_node;

 public:
  void setDebug(bool b) {
    debug_flag_arrow = b;
    auto node = remote_node.lock();
    if (node) node->debug_flag = b;
  }
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DETAIL_TARROWV1_HPP_
