#pragma once

// C++
#include <utility>
#include <vector>
//
#include <cycles/cycles_ptr.hpp>
//
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

static int mylistnode_count = 0;

class MyList {
 private:
  class MyListNode {
   public:
    double val;
    cycles_ptr<MyListNode> next;
    cycles_ptr<MyListNode> prev;
    MyListNode(double _val, cycles_ptr<MyListNode> _next,
               cycles_ptr<MyListNode> _prev)
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

  sptr<cycles_ctx<MyListNode>> ctx;

 public:
  bool debug_flag{false};
  cycles_ptr<MyListNode> entry;

  MyList() : ctx{new cycles_ctx<MyListNode>{}}, entry{make_null_node()} {}

  ~MyList() {
    if (debug_flag) std::cout << "~MyList" << std::endl;
    ctx = nullptr;
    // entry.do_reset();
  }

  // NOLINTNEXTLINE
  void addNext(double v, cycles_ptr<MyListNode>& node) {
    node->next = make_node_owned(v, node);
    node->next->prev = node.copy_owned(node->next);
  }

  // HELPERS FOR CTX

  auto my_ctx() -> wptr<cycles_ctx<MyListNode>> { return this->ctx; }

  auto make_node(double v) -> cycles_ptr<MyListNode> {
    auto* ptr =
        new MyListNode(v, make_null_node(), make_null_node());  // NOLINT
    return cycles_ptr<MyListNode>(this->ctx, ptr);
  }

  auto make_node_owned(double v, const cycles_ptr<MyListNode>& owner)
      -> cycles_ptr<MyListNode> {
    auto ptr1 = cycles_ptr<MyListNode>(
        this->ctx, new MyListNode(v, make_null_node(), make_null_node()));
    return ptr1.copy_owned(owner);
  }

  auto make_null_node() -> cycles_ptr<MyListNode> {
    return cycles_ptr<MyListNode>(this->ctx, nullptr);
  }

  // PRINT

  void print() {
    std::cout << "============================ " << std::endl
              << "MyList::print() => root.has_get() = "
              << (entry.has_get() ? "true" : "false") << std::endl;
    if (entry.has_get()) printFrom(entry);
    std::cout << "MyList::finished PRINT" << std::endl;
    std::cout << "============================ " << std::endl;
  }

  void printFrom(const cycles_ptr<MyListNode>& node) {
    if (node.has_get()) {
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
