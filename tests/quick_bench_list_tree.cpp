
//
#include <cmath>
//
#include <chrono>
#include <iostream>
#include <queue>
#include <utility>
//
#include <cycles/cycles_ptr.hpp>

#include "TestList.hpp"
#include "TestTree.hpp"

// inspired from random bench for gcpp and tracked_ptr discussions

int main() {
  using namespace std::chrono;  // NOLINT
  using namespace cycles;       // NOLINT

  std::cout << "populate UList with unique_ptr" << std::endl;
  auto c = high_resolution_clock::now();
  {
    UList list;

    int nMax = 100'000;
    // SEGFAULT
    // int nMax = 10'000'000;
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new UListNode{.v = n++};  // NOLINT
    list.entry = std::unique_ptr<UListNode>{node};
    //
    while (n < nMax) {
      auto* node_next = new UListNode{.v = n++};  // NOLINT
      node->next = std::unique_ptr<UListNode>{node_next};
      node = node_next;
    }
    // std::cout << "will destroy list!" << std::endl;
  }
  std::cout
      << "UList unique_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =======================================

  std::cout << "populate SList with unique_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    SList list;

    int nMax = 100'000;
    // SEGFAULT
    // int nMax = 10'000'000;
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new SListNode{.v = n++};  // NOLINT
    list.entry = std::shared_ptr<SListNode>{node};
    //
    while (n < nMax) {
      auto* node_next = new SListNode{.v = n++};  // NOLINT
      node->next = std::shared_ptr<SListNode>{node_next};
      node = node_next;
    }
    // std::cout << "will destroy list!" << std::endl;
  }
  std::cout
      << "SList shared_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =======================================

  std::cout << "populate CList with cycles_ptr. Currently too slow! around 3 "
               "minutes... =("
            << std::endl;
  c = high_resolution_clock::now();
  if (false) {
    CList list;
    list.ctx = sptr<cycles_ctx>{new cycles_ctx{}};

    int nMax = 100'000;
    //
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node = new CListNode{.v = n++};  // NOLINT
      list.entry = cycles::cycles_ptr<CListNode>{list.ctx, node};
    }
    //
    cycles::cycles_ptr<CListNode>* current = &list.entry;
    //
    while (n < nMax) {
      // if (n % 1000) std::cout << "n=" << n << std::endl;
      auto* node_next = new CListNode{.v = n++};  // NOLINT
      (*current)->next =
          cycles::cycles_ptr<CListNode>{list.ctx, node_next}.copy_owned(
              *current);
      current = &((*current)->next);
    }
    // std::cout << "will destroy list!" << std::endl;
  }
  std::cout
      << "CList cycles_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate CList with cycles_ptr. no auto_collect! too terrible "
               "efficiency for now, ignore... "
            << std::endl;
  c = high_resolution_clock::now();
  if (false) {
    CList list;
    list.ctx = sptr<cycles_ctx>{new cycles_ctx{}};
    list.ctx->auto_collect = false;

    int nMax = 100'000;
    //
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node = new CListNode{.v = n++};  // NOLINT
      list.entry = cycles::cycles_ptr<CListNode>{list.ctx, node};
    }
    //
    cycles::cycles_ptr<CListNode>* current = &list.entry;
    //
    while (n < nMax) {
      // if (n % 1000) std::cout << "n=" << n << std::endl;
      auto* node_next = new CListNode{.v = n++};  // NOLINT
      (*current)->next =
          cycles::cycles_ptr<CListNode>{list.ctx, node_next}.copy_owned(
              *current);
      current = &((*current)->next);
    }
    // std::cout << "will destroy list!" << std::endl;
  }
  std::cout
      << "CList no auto_collect cycles_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with unique_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<UTreeNode*> temp;
    UTree tree;
    // DO NOT PUT 2^29... too much memory!
    int nMax = ::pow(2, 15) - 1;  // 10000000;
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new UTreeNode{.v = n++};  // NOLINT
    tree.root = std::unique_ptr<UTreeNode>{node};
    temp.push(tree.root.get());
    //
    int level = 1;
    while (n < nMax) {
      std::cout << "level=" << level << std::endl;
      assert(level == temp.size());
      //
      std::queue<UTreeNode*> newChild;
      for (unsigned j = 0; j < level; j++) {
        auto* target = temp.front();
        // std::cout << "processing target=" << target->v << std::endl;
        temp.pop();
        // binary tree
        auto* node1 = new UTreeNode{.v = n++};  // NOLINT
        target->children.push_back(std::unique_ptr<UTreeNode>{node1});
        //
        auto* node2 = new UTreeNode{.v = n++};  // NOLINT
        target->children.push_back(std::unique_ptr<UTreeNode>{node2});
        //
        newChild.push(node1);
        newChild.push(node2);
      }
      assert(newChild.size() == level * 2);
      temp = std::move(newChild);
      //
      level *= 2;
    }
    //
    // tree.root->nprint();
    //
    std::cout << "tHeight = " << tree.tHeight() << std::endl;
    std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "UTree with unique_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with shared_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<STreeNode*> temp;
    STree tree;
    // DO NOT PUT 2^29... too much memory!
    int nMax = ::pow(2, 15) - 1;  // 10000000;
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new STreeNode{.v = n++};  // NOLINT
    tree.root = std::shared_ptr<STreeNode>{node};
    temp.push(tree.root.get());
    //
    int level = 1;
    while (n < nMax) {
      std::cout << "level=" << level << std::endl;
      assert(level == temp.size());
      //
      std::queue<STreeNode*> newChild;
      for (unsigned j = 0; j < level; j++) {
        auto* target = temp.front();
        // std::cout << "processing target=" << target->v << std::endl;
        temp.pop();
        // binary tree
        auto* node1 = new STreeNode{.v = n++};  // NOLINT
        target->children.push_back(std::shared_ptr<STreeNode>{node1});
        //
        auto* node2 = new STreeNode{.v = n++};  // NOLINT
        target->children.push_back(std::shared_ptr<STreeNode>{node2});
        //
        newChild.push(node1);
        newChild.push(node2);
      }
      assert(newChild.size() == level * 2);
      temp = std::move(newChild);
      //
      level *= 2;
    }
    //
    // tree.root->nprint();
    //
    std::cout << "tHeight = " << tree.tHeight() << std::endl;
    std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "STree with shared_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with cycles_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<cycles_ptr<CTreeNode>*> temp;
    CTree tree;
    tree.ctx = sptr<cycles_ctx>{new cycles_ctx{}};
    // DO NOT PUT 2^29... too much memory!
    int nMax = ::pow(2, 15) - 1;  // 10000000;
    int n = 0;
    std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node = new CTreeNode{.v = n++};  // NOLINT
      tree.root = cycles::cycles_ptr<CTreeNode>{tree.ctx, node};
    }
    temp.push(&tree.root);
    //
    int level = 1;
    while (n < nMax) {
      std::cout << "level=" << level << std::endl;
      assert(level == temp.size());
      //
      std::queue<cycles_ptr<CTreeNode>*> newChild;
      for (unsigned j = 0; j < level; j++) {
        auto* target = temp.front();
        // std::cout << "processing target=" << target->v << std::endl;
        temp.pop();
        // binary tree
        auto* node1 = new CTreeNode{.v = n++};  // NOLINT
        (*target)->children.push_back(
            cycles_ptr<CTreeNode>{tree.ctx, node1}.copy_owned(*target));
        //
        auto* node2 = new CTreeNode{.v = n++};  // NOLINT
        (*target)->children.push_back(
            cycles_ptr<CTreeNode>{tree.ctx, node2}.copy_owned(*target));
        //
        newChild.push(&(*target)->children[0]);
        newChild.push(&(*target)->children[1]);
      }
      assert(newChild.size() == level * 2);
      temp = std::move(newChild);
      //
      level *= 2;
    }
    //
    // tree.root->nprint();
    //
    std::cout << "tHeight = " << tree.tHeight() << std::endl;
    std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "CTree with cycles_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  return 0;
}
