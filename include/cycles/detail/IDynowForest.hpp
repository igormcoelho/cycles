// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_DETAIL_IDYNOWFOREST_HPP_  // NOLINT
#define CYCLES_DETAIL_IDYNOWFOREST_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <utility>
#include <vector>

//
#include <cycles/detail/TNode.hpp>
#include <cycles/detail/TNodeData.hpp>
#include <cycles/detail/Tree.hpp>
#include <cycles/detail/utils.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// ======================================
// Interface for Dynamic Ownership Forest
// IDynowForest

namespace cycles {

namespace detail {

template <class XNode, class XTree, class XArrow>
class IDynowForest {
 public:
  using DynowNodeType = XNode;
  using DynowTreeType = XTree;
  using DynowArrowType = XArrow;
  // using DynowArrowType = std::pair<wptr<XNode>, wptr<XNode>>;

  virtual ~IDynowForest() = default;

  // debug helpers
  virtual void setDebug(bool b) {}
  virtual bool debug() { return false; }
  virtual void print() {}
  // base methods or not?
  virtual void collect() = 0;
  virtual bool autoCollect() { return true; }
  virtual void setAutoCollect(bool ac) {}
  // helpers
  virtual int getForestSize() = 0;  // testing only?
  virtual bool opx_hasParent(sptr<DynowNodeType> node_ptr) = 0;
  virtual int opx_countOwnedBy(sptr<DynowNodeType>) = 0;  // useless?
  virtual sptr<DynowNodeType> opx_getOwnedBy(sptr<DynowNodeType>,
                                             int idx) = 0;  // useless?
                                                            // main methods
  // op1: give 'data' and get arrow type (two weak pointers)
  virtual DynowArrowType op1_addNodeToNewTree(sptr<TNodeData>) = 0;
  // op2: give 'node locator' (weak or strong?) and 'data... returns 'arc'
  virtual DynowArrowType op2_addChildStrong(sptr<DynowNodeType>,
                                            sptr<TNodeData>) = 0;
  // op3: give two 'node locators' and get 'arc'
  virtual DynowArrowType op3_weakSetOwnedBy(sptr<DynowNodeType>,
                                            sptr<DynowNodeType>) = 0;
  // op4: give 'arc' reference (to clean it) and no return (void)
  // NOLINTNEXTLINE
  virtual void op4_remove(DynowArrowType& arrow, bool isRoot, bool isOwned) = 0;
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DETAIL_IDYNOWFOREST_HPP_ // NOLINT
