#pragma once

#include "cycles_ptr.hpp"
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

static int mynode_count = 0;

template <typename X>
class MyNode {
 public:
  MyNode(X _val) : val{_val} {
    mynode_count++;
    std::cout << "MyNode mynode_count=" << mynode_count << std::endl;
  }

  ~MyNode() {
    mynode_count--;
    std::cout << "~MyNode mynode_count=" << mynode_count << std::endl;
  }

  X val;

  vector<cycles_ptr<MyNode>> neighbors;
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

 private:
  sptr<cycles_ctx<MyNodeX>> ctx;

 public:
  auto my_ctx() -> wptr<cycles_ctx<MyNodeX>> { return this->ctx; }

  auto make_node(X v) -> cycles_ptr<MyNodeX> {
    auto* ptr = new MyNodeX(v);
    return cycles_ptr<MyNodeX>(this->ctx, ptr);
  }

  auto make_node_owned(X v, cycles_ptr<MyNodeX>& owner) -> cycles_ptr<MyNodeX> {
    return cycles_ptr<MyNodeX>(this->ctx, new MyNodeX(v), owner);
  }

  auto make_null_node() -> cycles_ptr<MyNodeX> {
    return cycles_ptr<MyNodeX>(this->ctx, nullptr);
  }

  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  cycles_ptr<MyNodeX> entry;

  MyGraph() : ctx{new cycles_ctx<MyNodeX>{}}, entry{make_null_node()} {}

  ~MyGraph() {
    std::cout << "~MyGraph" << std::endl;
    ctx = nullptr;
    // entry.do_reset();
  }

  void print() {
    std::cout << std::endl
              << "MyGraph::print() => root.has_get() = "
              << (entry.has_get() ? "true" : "false") << std::endl;
    std::cout << "MyGraph::ctx (pointer) ~> " << ctx << std::endl;
    if (ctx) ctx->print();
    std::cout << "MyGraph::finished ctx print" << std::endl;
    if (entry.has_get()) printFrom(entry);
    std::cout << "MyGraph::finished PRINT" << std::endl;
    std::cout << std::endl;
  }

  void printFrom(cycles_ptr<MyNodeX> node) {
    if (node.has_get()) {
      std::cout << "node=" << node.get()
                << " |neighbors|=" << node.get().neighbors.size() << std::endl;

      for (unsigned i = 0; i < node.get().neighbors.size(); i++) {
        if (node.get().neighbors[i] == entry) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else
          printFrom(node.get().neighbors[i]);
      }
    }
  }
  //
};