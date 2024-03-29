
//
#include <cmath>
//
#include <chrono>
#include <iostream>
#include <queue>
#include <thread>  // this_thread
#include <utility>
//
#include <cycles/relation_ptr.hpp>

#include "../TestList.hpp"
#include "../TestTree.hpp"

// inspired from random bench for gcpp and tracked_ptr discussions

int main() {
  using namespace std::chrono;  // NOLINT
  using namespace cycles;       // NOLINT

#ifdef BENCH_LONG_DEFERRED
  std::cout << "BENCH_LONG_DEFERRED=1" << std::endl;
  constexpr int nMaxList = 10'000'000;
#else
  constexpr int nMaxList = 100'000;
#endif
  //
  int nMaxTree = ::pow(2, 15) - 1;
  //
  std::cout << "nMaxList=" << nMaxList << std::endl;
  std::cout << "nMaxTree=" << nMaxTree << std::endl;

  std::cout << "populate UList with unique_ptr" << std::endl;
  auto c = high_resolution_clock::now();
  {
    UList list;

    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new UListNode{.v = n++};  // NOLINT
    list.entry = std::unique_ptr<UListNode>{node};
    //
    while (n < nMaxList) {
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

    // SEGFAULT
    // int nMax = 10'000'000;
    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new SListNode{.v = n++};  // NOLINT
    list.entry = std::shared_ptr<SListNode>{node};
    //
    while (n < nMaxList) {
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

  std::cout << "populate CList with relation_ptr" << std::endl;
  c = high_resolution_clock::now();
  if (true) {
    CList list;
    // list.ctx = sptr<DynowForestV1>{new DynowForestV1{}};

    //
    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node = new CListNode{.v = n++};  // NOLINT
      list.entry = cycles::relation_ptr<CListNode>{node, list.pool};
    }
    //
    cycles::relation_ptr<CListNode>* current = &list.entry;
    //
    while (n < nMaxList) {
      // if (n % 1000) std::cout << "n=" << n << std::endl;
      auto* node_next = new CListNode{.v = n++};  // NOLINT

      // (*current)->next =
      //    cycles::relation_ptr<CListNode>{list.ctx, node_next}.get_owned(
      //        *current);

      (*current)->next = cycles::relation_ptr<CListNode>{node_next, *current};

      current = &((*current)->next);
    }
    // std::cout << "will destroy list!" << std::endl;
  }
  std::cout
      << "CList relation_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate CList with relation_ptr. no auto_collect!"
            << std::endl;
  c = high_resolution_clock::now();
  if (true) {
    CList list;
    // list.ctx = sptr<DynowForestV1>{new DynowForestV1{}};
    list.pool.getContext()->setAutoCollect(false);

    //
    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node_ptr = new CListNode{.v = n++};  // NOLINT
      list.entry = cycles::relation_ptr<CListNode>{node_ptr, list.pool};
    }
    //
    cycles::relation_ptr<CListNode>* current = &list.entry;
    //
    while (n < nMaxList) {
      // if (n % 1000) std::cout << "n=" << n << std::endl;
      auto* node_next = new CListNode{.v = n++};  // NOLINT
#if 0
      (*current)->next =
          cycles::relation_ptr<CListNode>{list.ctx, node_next}.get_owned(
              *current);
#else
      (*current)->next = cycles::relation_ptr<CListNode>{node_next, *current};
#endif

      current = &((*current)->next);
    }
    // std::cout << "will destroy list!" << std::endl;
    // list.ctx->collect();
  }
  std::cout
      << "CList no auto_collect relation_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with unique_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<UTreeNode*> temp;
    UTree tree;
    // DO NOT PUT 2^29... too much memory!

    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new UTreeNode{.v = n++};  // NOLINT
    tree.root = std::unique_ptr<UTreeNode>{node};
    temp.push(tree.root.get());
    //
    int level = 1;
    while (n < nMaxTree) {
      // std::cout << "level=" << level << std::endl;
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
    // std::cout << "tHeight = " << tree.tHeight() << std::endl;
    // std::cout << "will destroy tree!" << std::endl;
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

    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    auto* node = new STreeNode{.v = n++};  // NOLINT
    tree.root = std::shared_ptr<STreeNode>{node};
    temp.push(tree.root.get());
    //
    int level = 1;
    while (n < nMaxTree) {
      // std::cout << "level=" << level << std::endl;
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
    // std::cout << "tHeight = " << tree.tHeight() << std::endl;
    // std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "STree with shared_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with relation_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<relation_ptr<CTreeNode>*> temp;
    CTree tree;
    // tree.ctx = sptr<DynowForestV1>{new DynowForestV1{}};
    // DO NOT PUT 2^29... too much memory!

    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node = new CTreeNode{.v = n++};  // NOLINT
      tree.root = cycles::relation_ptr<CTreeNode>{node, tree.pool};
    }
    temp.push(&tree.root);
    //
    int level = 1;
    while (n < nMaxTree) {
      // std::cout << "level=" << level << std::endl;
      assert(level == temp.size());
      //
      std::queue<relation_ptr<CTreeNode>*> newChild;
      for (unsigned j = 0; j < level; j++) {
        auto* target = temp.front();
        // std::cout << "processing target=" << target->v << std::endl;
        temp.pop();
        // binary tree
        auto* node1 = new CTreeNode{.v = n++};  // NOLINT

#if 0
        (*target)->children.push_back(
            relation_ptr<CTreeNode>{tree.ctx, node1}.get_owned(*target));
#else
        (*target)->children.push_back(relation_ptr<CTreeNode>{node1, *target});
#endif
        //
        auto* node2 = new CTreeNode{.v = n++};  // NOLINT

#if 0
        (*target)->children.push_back(
            relation_ptr<CTreeNode>{tree.ctx, node2}.get_owned(*target));
#else
        (*target)->children.push_back(relation_ptr<CTreeNode>{node2, *target});
#endif
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
    // std::cout << "tHeight = " << tree.tHeight() << std::endl;
    // std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "CTree with relation_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  std::cout << "populate TestTree with relation_ptr - no auto_collect"
            << std::endl;
  c = high_resolution_clock::now();
  {
    std::queue<relation_ptr<CTreeNode>*> temp;
    CTree tree;
    // tree.ctx = sptr<DynowForestV1>{new DynowForestV1{}};
    tree.pool.getContext()->setAutoCollect(false);
    // DO NOT PUT 2^29... too much memory!

    int n = 0;
    // std::cout << "nMax=" << nMax << std::endl;
    // initialize root
    {
      auto* node_ptr = new CTreeNode{.v = n++};  // NOLINT
      tree.root = cycles::relation_ptr<CTreeNode>{node_ptr, tree.pool};
    }
    temp.push(&tree.root);
    //
    int level = 1;
    while (n < nMaxTree) {
      // std::cout << "level=" << level << std::endl;
      assert(level == temp.size());
      //
      std::queue<relation_ptr<CTreeNode>*> newChild;
      for (unsigned j = 0; j < level; j++) {
        auto* target = temp.front();
        // std::cout << "processing target=" << target->v << std::endl;
        temp.pop();
        // binary tree
        auto* node1 = new CTreeNode{.v = n++};  // NOLINT

#if 0
        (*target)->children.push_back(
            relation_ptr<CTreeNode>{tree.ctx, node1}.get_owned(*target));
#else
        (*target)->children.push_back(relation_ptr<CTreeNode>{node1, *target});
#endif
        //
        auto* node2 = new CTreeNode{.v = n++};  // NOLINT

#if 0
        (*target)->children.push_back(
            relation_ptr<CTreeNode>{tree.ctx, node2}.get_owned(*target));
#else
        (*target)->children.push_back(relation_ptr<CTreeNode>{node2, *target});
#endif
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
    // std::cout << "tHeight = " << tree.tHeight() << std::endl;
    // std::cout << "will destroy tree!" << std::endl;
  }
  std::cout
      << "CTree with relation_ptr - no auto_collect: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // ================================

  return 0;
}
