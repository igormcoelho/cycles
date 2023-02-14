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
#include <cycles/detail/TNodeData.hpp>
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TNodeV1.hpp>
#include <cycles/detail/v1/TreeV1.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// =======================================
// Interface for Dynamic Ownership Forest:
// IDynowForest

namespace cycles {

namespace detail {

// prefer concept instead of this interface
class IArrow {
 public:
  virtual ~IArrow() = default;
  virtual bool is_null() const = 0;
  virtual bool is_root() const = 0;
  virtual bool is_owned() const = 0;
};

#if __cplusplus > 201703L  // c++20 supported
template <class T>
concept XArrowType = requires(T self, bool b) {
  typename T::data_type;  // default is sptr<TNodeData>
  { self.is_null() } -> std::convertible_to<bool>;
  { self.is_root() } -> std::convertible_to<bool>;
  { self.is_owned() } -> std::convertible_to<bool>;
  // NOLINTNEXTLINE
};
#endif

#if __cplusplus > 201703L  // c++20 supported
template <XArrowType XArrow>
#else
template <class XArrow>
#endif
class IDynowForest {
 public:
  using DynowArrowType = XArrow;
  // Default data type is sptr<TNodeData>
  // It must be the same as XArrow::data_type
  using DynowDataType = typename XArrow::data_type;
  //
  virtual ~IDynowForest() = default;

  // debug helpers
  virtual void setDebug(bool b) {}
  virtual bool debug() { return false; }
  virtual void print() {}
  // base methods or not?
  virtual void collect() = 0;
  virtual bool getAutoCollect() { return true; }
  // NOTE 1: setAutoCollect will return 'false' if not supported
  // NOTE 2: setAutoCollect should collect() if 'true' is passed (even
  // if not supported!)
  virtual bool setAutoCollect(bool ac) {
    if (ac) collect();
    return false;
  }
  // helpers
  virtual int getForestSize() = 0;  // testing only?
  //
  // op0: get type-erased data from arrow as shared_ptr
  virtual DynowDataType op0_getSharedData(const DynowArrowType& arc) = 0;
  // op1: give 'data' and get arrow type (two weak pointers)
  virtual DynowArrowType op1_addNodeToNewTree(DynowDataType) = 0;
  // op2: give 'node locator' (weak or strong?) and 'data... returns 'arc'
  // virtual DynowArrowType op2_addChildStrong(sptr<DynowNodeType>,
  //                                           sptr<TNodeData>) = 0;
  virtual DynowArrowType op2_addChildStrong(const DynowArrowType&,
                                            DynowDataType) = 0;

  // op3: give two 'node locators' and get 'arc'
  // virtual DynowArrowType op3_weakSetOwnedBy(sptr<DynowNodeType> owned,
  //                                          sptr<DynowNodeType> owner) = 0;
  virtual DynowArrowType op3_weakSetOwnedBy(const DynowArrowType& owned,
                                            const DynowArrowType& owner) = 0;

  // op4: give 'arc' reference (to clean it) and no return (void)
  // NOLINTNEXTLINE
  virtual void op4_remove(DynowArrowType& arrow) = 0;

  // op5: receive 'arc' and make 'unowned' link
  virtual DynowArrowType op5_copyNodeToNewTree(const DynowArrowType& arrow) = 0;
  // cleanup method (for pool)
  virtual void destroyAll() = 0;
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DETAIL_IDYNOWFOREST_HPP_ // NOLINT
