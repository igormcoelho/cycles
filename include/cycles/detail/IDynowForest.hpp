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

template <class XNode, class XTree>
class IDynowForest {
 public:
  using DynowNodeType = XNode;
  using DynowTreeType = XTree;

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
  virtual void op1_addNodeToNewTree(sptr<DynowNodeType>) = 0;
  /*
  virtual void op2_addChildStrong(sptr<DynowNodeType>, sptr<DynowNodeType>) = 0;
  virtual void op3_weakSetOwnedBy(sptr<DynowNodeType>, sptr<DynowNodeType>) = 0;
  virtual void op4_remove(sptr<DynowNodeType>, sptr<DynowNodeType>, bool,
                          bool) = 0;
                          */
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DETAIL_IDYNOWFOREST_HPP_ // NOLINT
