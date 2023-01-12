#pragma once

// C++
#include <vector>
//
#include <cycles/cycles_ptr.hpp>
//
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

inline int mynode_count = 0;

template <typename X>
class MyNode {
 public:
  X val;
  vector<cycles_ptr<MyNode>> neighbors;
  bool debug_flag{false};

  explicit MyNode(X _val, bool _debug_flag = false)
      : val{_val}, debug_flag{_debug_flag} {
    mynode_count++;
    if (debug_flag)
      std::cout << "MyNode mynode_count=" << mynode_count << std::endl;
  }

  ~MyNode() {
    mynode_count--;
    if (debug_flag) {
      std::cout << "~MyNode(" << val << ") mynode_count=" << mynode_count
                << std::endl;
      std::cout << "~MyNode: |neighbors|=" << neighbors.size() << std::endl;
      if (neighbors.size() > 0) {
        std::cout << "~MyNode: WILL CLEAR MY NEIGHBORS!" << std::endl;
        for (unsigned i = 0; i < neighbors.size(); i++)
          std::cout << "  MyNode neighbor i=" << i
                    << " => type: " << neighbors[i].getType() << std::endl;
        neighbors.clear();
      }
      std::cout << "~MyNode(" << val << "): FINISHED!" << std::endl;
    }
  }
  //
  friend ostream& operator<<(ostream& os, const MyNode& node) {
    os << "MyNode(" << node.val << ")";
    return os;
  }
};

// ---------

template <typename X>
class MyGraph {
  using MyNodeX = MyNode<X>;

 public:
  bool debug_flag{false};

 private:
  sptr<cycles_ctx> ctx;

 public:
  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  cycles_ptr<MyNodeX> entry;

  MyGraph() : ctx{new cycles_ctx{}}, entry{make_null_node()} {}

  ~MyGraph() {
    if (debug_flag) std::cout << "~MyGraph" << std::endl;
    ctx = nullptr;
    // entry.do_reset();
  }
  //
  auto my_ctx() -> wptr<cycles_ctx> { return this->ctx; }

  auto make_node(X v) -> cycles_ptr<MyNodeX> {
    auto* ptr = new MyNodeX(v, debug_flag);  // NOLINT
    int nc1 = tnode_count;
    cycles_ptr<MyNodeX> cptr(this->ctx, ptr);
    int nc2 = tnode_count;
    // checking tnode_count against possible (and crazy...) ODR errors
    assert(nc2 == nc1 + 1);
    return cptr;
  }

  auto make_node_owned(X v, const cycles_ptr<MyNodeX>& owner)
      -> cycles_ptr<MyNodeX> {
    auto ptr1 = cycles_ptr<MyNodeX>(this->ctx, new MyNodeX(v, debug_flag));
    return ptr1.copy_owned(owner);
    // return cycles_ptr<MyNodeX>(this->ctx, new MyNodeX(v, debug_flag), owner);
  }

  auto make_null_node() -> cycles_ptr<MyNodeX> {
    return cycles_ptr<MyNodeX>(this->ctx, nullptr);
  }

  void print() {
    std::cout << "============================ " << std::endl
              << "MyGraph::print() => root.has_get() = "
              << (entry.has_get() ? "true" : "false") << std::endl;
    // std::cout << "MyGraph::ctx (pointer) ~> " << ctx << std::endl;
    // if (ctx) ctx->print();
    // std::cout << "MyGraph::finished ctx print" << std::endl;
    if (entry.has_get()) printFrom(entry);
    std::cout << "MyGraph::finished PRINT" << std::endl;
    std::cout << "============================ " << std::endl;
  }

  void printFrom(const cycles_ptr<MyNodeX>& node) {
    if (node.has_get()) {
      std::cout << "node=" << node.get()
                << " |neighbors|=" << node.get().neighbors.size() << std::endl;

      for (unsigned i = 0; i < node.get().neighbors.size(); i++) {
        if (node.get().neighbors[i] == entry) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else {
          printFrom(node.get().neighbors[i]);
        }
      }
    }
  }
  //
};
