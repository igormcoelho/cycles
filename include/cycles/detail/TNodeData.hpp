// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_TNODEDATA_HPP_  // NOLINT
#define CYCLES_TNODEDATA_HPP_  // NOLINT

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

using std::ostream, std::vector;  // NOLINT

// ======================================
// Helper to detect if type is shared_ptr
//
template <class X>
struct is_shared_ptr : std::false_type {};
template <class X>
struct is_shared_ptr<std::shared_ptr<X>> : std::true_type {};
// ======================================

// ============================
//   memory-managed Tree Node
// ============================
// Tree Node
// all memory is self-managed

namespace cycles {

namespace detail {

// TNodeData is inspired by Herb Sutter's gcpp 'struct destructor{...}'
// It uses lambda functions to perform type erasure

class TNodeData {
 public:
  // raw pointer to the object
  const void* p;
  // raw pointer to a type-erased destructor function
  void (*destroy)(const void*);

#ifdef CYCLES_TOSTRING
  // debug only: raw pointer to a type-erased toString function
  std::string (*toString)(const void*);
#endif

#ifdef CYCLES_TOSTRING
  TNodeData(void* _p, void (*_destroy)(const void*),
            std::string (*_toString)(const void*))
      : p{_p}, destroy{_destroy}, toString{_toString} {}
#else
  TNodeData(void* _p, void (*_destroy)(const void*))
      : p{_p}, destroy{_destroy} {}
#endif

  // no copy allowed here
  TNodeData(const TNodeData&) = delete;

  // move is allowed
#ifdef CYCLES_TOSTRING
  TNodeData(TNodeData&& corpse) noexcept
      : p{corpse.p}, destroy{corpse.destroy}, toString{corpse.toString} {}
#else
  TNodeData(TNodeData&& corpse) noexcept
      : p{corpse.p}, destroy{corpse.destroy} {}
#endif

  ~TNodeData() {
    // std::cout << "~TNodeData(" << toString(p) << ")" << std::endl;
    destroy(p);
    p = 0;
  }

  friend std::ostream& operator<<(std::ostream& os, const TNodeData& me) {
#ifdef CYCLES_TOSTRING
    os << "TNodeData(" << me.toString(me.p) << ")";
#else
    os << "TNodeData(undefined CYCLES_TOSTRING)";
#endif

    return os;
  }

  template <class T>
  static TNodeData make(T* ptr) {
    TNodeData data{ptr,
                   [](const void* x) {
                     // T destructor to invoke
                     //
                     // DOES IT WORK FOR PRIMITIVE TYPES?
                     //
                     // static_cast<const T*>(x)->~T();
                     //
                     // NOLINTNEXTLINE
                     if (x) delete static_cast<const T*>(x);
                   }
#ifdef CYCLES_TOSTRING
                   ,
                   [](const void* x) {
                     std::stringstream ss;
                     if (x)
                       ss << *static_cast<const T*>(x);
                     else
                       ss << "NULL";
                     return ss.str();
                   }
#endif
    };
    return data;
  }

  template <class T>
  static sptr<TNodeData> make_sptr(T* ptr) {
    if constexpr (std::is_void<T>::value) {
      // NOLINTNEXTLINE
      TNodeData* data = nullptr;
      return sptr<TNodeData>{data};
    } else {
      // NOLINTNEXTLINE
      auto* data = new TNodeData{ptr,
                                 [](const void* x) {
                                   // T destructor to invoke
                                   //
                                   // DOES IT WORK FOR PRIMITIVE TYPES?
                                   //
                                   // static_cast<const T*>(x)->~T();
                                   //
                                   // NOLINTNEXTLINE
                                   if (x) delete static_cast<const T*>(x);
                                 }
#ifdef CYCLES_TOSTRING
                                 ,
                                 [](const void* x) {
                                   std::stringstream ss;
                                   if (x)
                                     ss << *static_cast<const T*>(x);
                                   else
                                     ss << "NULL";
                                   return ss.str();
                                 }
#endif
      };

      return sptr<TNodeData>{data};
    }
  }
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_TNODEDATA_HPP_ // NOLINT
