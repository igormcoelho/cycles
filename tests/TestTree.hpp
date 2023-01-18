#pragma once

#include <iostream>
#include <memory>
#include <vector>
//
#include <cycles/relation_ptr.hpp>

namespace cycles {

// =========================
//       unique_ptr
// =========================

class UTreeNode {
 public:
  int v;
  std::vector<std::unique_ptr<UTreeNode>> children;

  int height() const {
    int hMax = 0;
    for (auto i = 0; i < children.size(); i++) {
      int h = 0;
      if (children[i]) h = children[i]->height();
      if (h > hMax) hMax = h;
    }
    return hMax + 1;
  }

  void nprint() const {
    std::cout << v << std::endl;
    for (auto i = 0; i < children.size(); i++)
      if (children[i]) children[i]->nprint();
  }
};

class UTree {
 public:
  std::unique_ptr<UTreeNode> root;

  int tHeight() const {
    if (!root)
      return 0;
    else
      return root->height();
  }

  ~UTree() {
    // std::cout << "~UTree BEGIN" << std::endl;
    root = nullptr;
    // std::cout << "~UTree END" << std::endl;
  }
};

// =========================
//       shared_ptr
// =========================

class STreeNode {
 public:
  int v;
  std::vector<std::shared_ptr<STreeNode>> children;

  int height() const {
    int hMax = 0;
    for (auto i = 0; i < children.size(); i++) {
      int h = 0;
      if (children[i]) h = children[i]->height();
      if (h > hMax) hMax = h;
    }
    return hMax + 1;
  }

  void nprint() const {
    std::cout << v << std::endl;
    for (auto i = 0; i < children.size(); i++)
      if (children[i]) children[i]->nprint();
  }
};

class STree {
 public:
  std::shared_ptr<STreeNode> root;

  int tHeight() const {
    if (!root)
      return 0;
    else
      return root->height();
  }

  ~STree() {
    // std::cout << "~STree BEGIN" << std::endl;
    root = nullptr;
    // std::cout << "~STree END" << std::endl;
  }
};

// =========================
//       relation_ptr
// =========================

class CTreeNode {
 public:
  int v;
  std::vector<cycles::relation_ptr<CTreeNode>> children;

  int height() const {
    int hMax = 0;
    for (auto i = 0; i < children.size(); i++) {
      int h = 0;
      if (children[i].has_get()) h = children[i]->height();
      if (h > hMax) hMax = h;
    }
    return hMax + 1;
  }

  void nprint() const {
    std::cout << v << std::endl;
    for (auto i = 0; i < children.size(); i++)
      if (children[i].has_get()) children[i]->nprint();
  }

  friend std::ostream& operator<<(std::ostream& os, const CTreeNode& me) {
    os << "CTreeNode(" << me.v << ")";
    return os;
  }
};

class CTree {
 public:
  sptr<cycles::forest_ctx> ctx;
  cycles::relation_ptr<CTreeNode> root;

  int tHeight() const {
    if (!root.has_get())
      return 0;
    else
      return root->height();
  }

  ~CTree() {
    // std::cout << "~CTree BEGIN" << std::endl;
    root.reset();
    // std::cout << "~CTree END" << std::endl;
  }
};

}  // namespace cycles
