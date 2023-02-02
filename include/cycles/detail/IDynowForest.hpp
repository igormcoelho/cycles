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
  virtual void op2_addChildStrong(sptr<DynowNodeType>, sptr<DynowNodeType>) = 0;
  virtual void op3_weakSetOwnedBy(sptr<DynowNodeType>, sptr<DynowNodeType>) = 0;
  virtual void op4_remove(sptr<DynowNodeType> sptr_mynode,
                          sptr<DynowNodeType> owner_node, bool isRoot,
                          bool isOwned) = 0;
  // strange helpers for op4_remove
  virtual void opx_clearWeakLinks(sptr<DynowNodeType> owner_node,
                                  sptr<DynowNodeType> sptr_mynode) = 0;
  virtual bool opx_setNewOwner(sptr<DynowNodeType> sptr_mynode) = 0;
  virtual bool op4x_checkSituation(sptr<DynowNodeType> sptr_mynode,
                                   sptr<DynowNodeType> owner_node, bool isRoot,
                                   bool isOwned) = 0;
  // this is the buggy one!!
  virtual void op4x_clearAndCollect(bool will_die,
                                    sptr<DynowNodeType> sptr_mynode,
                                    sptr<DynowNodeType> owner_node, bool isRoot,
                                    bool isOwned) = 0;
  virtual void op4x_prepareDestruction(sptr<DynowNodeType> sptr_mynode,
                                       sptr<DynowNodeType> owner_node,
                                       bool isRoot, bool isOwned) = 0;
  virtual void op4x_destroyNode(sptr<DynowNodeType>& sptr_mynode) = 0;
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DETAIL_IDYNOWFOREST_HPP_ // NOLINT
