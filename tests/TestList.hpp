#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <vector>
//
#include <cycles/relation_ptr.hpp>

namespace cycles {

// =========================
//       unique_ptr
// =========================

class UListNode {
 public:
  int v;
  std::unique_ptr<UListNode> next;

#ifdef BENCH_LONG_DEFERRED
  ~UListNode() {
    for (std::unique_ptr<UListNode> current = std::move(next); current;
         current = std::move(current->next)) {
    }
  }
#endif
};

class UList {
 public:
  std::unique_ptr<UListNode> entry;

  ~UList() {
    // std::cout << "~UList BEGIN" << std::endl;
    entry = nullptr;
    // std::cout << "~UList END" << std::endl;
  }
};

// =========================
//       shared_ptr
// =========================

class SListNode {
 public:
  int v;
  std::shared_ptr<SListNode> next;

#ifdef BENCH_LONG_DEFERRED
  ~SListNode() {
    for (std::shared_ptr<SListNode> current = std::move(next); current;
         current = std::move(current->next)) {
    }
  }
#endif
};

class SList {
 public:
  std::shared_ptr<SListNode> entry;

  ~SList() {
    // std::cout << "~SList BEGIN" << std::endl;
    entry = nullptr;
    // std::cout << "~SList END" << std::endl;
  }
};

// =========================
//       relation_ptr
// =========================

class CListNode {
 public:
  int v;
  cycles::relation_ptr<CListNode> next;

  friend std::ostream& operator<<(std::ostream& os, const CListNode& me) {
    os << "CListNode(" << me.v << ")";
    return os;
  }
};

class CList {
 public:
  sptr<cycles::DynowForestV1> ctx;
  cycles::relation_ptr<CListNode> entry;

  ~CList() {
    // std::cout << "~CList BEGIN" << std::endl;
    entry.reset();
    // std::cout << "~CList END" << std::endl;
  }
};

}  // namespace cycles
