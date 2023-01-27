#pragma once

// C++
#include <utility>
#include <vector>
//
#include <cycles/relation_ptr.hpp>
//
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

static int mylistnode_count = 0;

class MyList {
 private:
  class MyListNode {
   public:
    double val;
    relation_ptr<MyListNode> next;
    relation_ptr<MyListNode> prev;
    MyListNode(double _val, relation_ptr<MyListNode> _next,
               relation_ptr<MyListNode> _prev)
        : val{_val}, next{std::move(_next)}, prev{std::move(_prev)} {
      mylistnode_count++;
    }
    ~MyListNode() { mylistnode_count--; }
    //
    friend ostream& operator<<(ostream& os, const MyListNode& node) {
      os << "MyListNode(" << node.val << ")";
      return os;
    }
  };

  sptr<forest_ctx> ctx;

 public:
  bool debug_flag{false};
  relation_ptr<MyListNode> entry;

  MyList() : ctx{new forest_ctx{}}, entry{make_null_node()} {}

  ~MyList() {
    if (debug_flag) std::cout << "~MyList" << std::endl;
    ctx = nullptr;
    // entry.do_reset();
  }

  // NOLINTNEXTLINE
  void addNext(double v, relation_ptr<MyListNode>& node) {
    node->next = make_node_owned(v, node);
    node->next->prev = node.get_owned(node->next);
  }

  // HELPERS FOR CTX

  auto my_ctx() -> wptr<forest_ctx> { return this->ctx; }

  auto make_node(double v) -> relation_ptr<MyListNode> {
    auto* ptr =
        new MyListNode(v, make_null_node(), make_null_node());  // NOLINT
    return relation_ptr<MyListNode>(this->ctx, ptr);
  }

  auto make_node_owned(double v, const relation_ptr<MyListNode>& owner)
      -> relation_ptr<MyListNode> {
    auto ptr1 = relation_ptr<MyListNode>(
        this->ctx, new MyListNode(v, make_null_node(), make_null_node()));
    return ptr1.get_owned(owner);
  }

  auto make_null_node() -> relation_ptr<MyListNode> {
    return relation_ptr<MyListNode>(this->ctx, nullptr);
  }

  // PRINT

  void print() {
    std::cout << "============================ " << std::endl
              << "MyList::print() => root.has_get() = "
              << (entry ? "true" : "false") << std::endl;
    if (entry) printFrom(entry);
    std::cout << "MyList::finished PRINT" << std::endl;
    std::cout << "============================ " << std::endl;
  }

  void printFrom(const relation_ptr<MyListNode>& node) {
    if (node) {
      std::cout << "node:" << node.get() << " prev:" << node->prev.get()
                << " next:" << node->prev.get() << std::endl;

      if (node->next == entry) {
        std::cout << "WARNING: cyclic list! stop printing..." << std::endl;
      } else {
        printFrom(node->next);
      }
    }
  }
  //
};
