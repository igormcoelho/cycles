#pragma once

#include "cycle_ptr.hpp"
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

  vector<cycle_ptr<MyNode>> neighbors;
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
  sptr<cycle_ctx<MyNodeX>> ctx;

 public:
  auto my_ctx() -> wptr<cycle_ctx<MyNodeX>> { return this->ctx; }

  auto make_node(X v) -> cycle_ptr<MyNodeX> {
    auto* ptr = new MyNodeX(v);
    return cycle_ptr<MyNodeX>(this->ctx, ptr);
  }

  auto make_node_owned(X v, cycle_ptr<MyNodeX>& owner) -> cycle_ptr<MyNodeX> {
    return cycle_ptr<MyNodeX>(this->ctx, new MyNodeX(v), owner);
  }

  auto make_null_node() -> cycle_ptr<MyNodeX> {
    return cycle_ptr<MyNodeX>(this->ctx, nullptr);
  }

  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  cycle_ptr<MyNodeX> entry;

  MyGraph() : entry{make_null_node()}, ctx{new cycle_ctx<MyNodeX>{}} {}

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

  void printFrom(cycle_ptr<MyNodeX> node) {
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