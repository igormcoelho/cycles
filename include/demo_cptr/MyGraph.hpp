#pragma once

// C++
#include <vector>
//
#include <cycles/relation_ptr.hpp>
//
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

inline int mynode_count = 0;

template <typename X>
class MyNode {
 public:
  X val;
  vector<relation_ptr<MyNode>> neighbors;
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
  // sptr<forest_ctx> ctx;
  relation_pool pool;

 public:
  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  relation_ptr<MyNodeX> entry;

  MyGraph() : entry{make_null_node()} {}

  ~MyGraph() {
    if (debug_flag) std::cout << "~MyGraph" << std::endl;
    pool.clear();
    // entry.reset();
  }
  //
  auto my_ctx() -> wptr<relation_pool::pool_type> {
    return this->pool.getContext();
  }

  auto make_node(X v) -> relation_ptr<MyNodeX> {
    auto* ptr = new MyNodeX(v, debug_flag);  // NOLINT
    int nc1 = tnode_count;
    relation_ptr<MyNodeX> cptr(this->pool.getContext(), ptr);
    int nc2 = tnode_count;
    // checking tnode_count against possible (and crazy...) ODR errors
    assert(nc2 == nc1 + 1);
    return cptr;
  }

  auto make_node_owned(X v, const relation_ptr<MyNodeX>& owner)
      -> relation_ptr<MyNodeX> {
#if 0
    auto ptr1 = relation_ptr<MyNodeX>(this->pool.getContext(),
                                      new MyNodeX(v, debug_flag));
    return ptr1.get_owned(owner);
#else
    return relation_ptr<MyNodeX>(this->pool.getContext(),
                                 new MyNodeX(v, debug_flag), owner);
#endif
  }

  auto make_null_node() -> relation_ptr<MyNodeX> {
    return relation_ptr<MyNodeX>(this->pool.getContext(), nullptr);
  }

  void print() {
    std::cout << "============================ " << std::endl
              << "MyGraph::print() => root.has_get() = "
              << (entry ? "true" : "false") << std::endl;
    // std::cout << "MyGraph::ctx (pointer) ~> " << ctx << std::endl;
    // if (ctx) ctx->print();
    // std::cout << "MyGraph::finished ctx print" << std::endl;
    if (entry) printFrom(entry);
    std::cout << "MyGraph::finished PRINT" << std::endl;
    std::cout << "============================ " << std::endl;
  }

  void printFrom(const relation_ptr<MyNodeX>& node) {
    if (node) {
      std::cout << "node=" << node.get()
                << " |neighbors|=" << node.get()->neighbors.size() << std::endl;

      for (unsigned i = 0; i < node.get()->neighbors.size(); i++) {
        if (node.get()->neighbors[i] == entry) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else {
          printFrom(node.get()->neighbors[i]);
        }
      }
    }
  }
  //
};

// ============= EXPERIMENTAL ==============

/*
//
// NO C[-1] Constructor!! Too buggy, risky (but likely beautiful!)
//
class MyNodeLoop {
 private:
  relation_ptr<MyNodeLoop> self;

 public:
  double val;
  vector<relation_ptr<MyNodeLoop>> neighbors;

  bool debug_flag{false};

  explicit MyNodeLoop(const relation_pool& pool, double _val,
                      bool _debug_flag = false)
      : self{relation_ptr<MyNodeLoop>(pool.getContext(), this,
                                      cycles::weak_self{})},
        val{_val},
        debug_flag{_debug_flag} {
    mynode_count++;
    if (debug_flag)
      std::cout << "MyNodeLoop mynode_count=" << mynode_count << std::endl;
  }

  ~MyNodeLoop() {
    mynode_count--;
    if (debug_flag) {
      std::cout << "~MyNodeLoop(" << val << ") mynode_count=" << mynode_count
                << std::endl;
      std::cout << "~MyNodeLoop: |neighbors|=" << neighbors.size() << std::endl;
      if (neighbors.size() > 0) {
        std::cout << "~MyNodeLoop: WILL CLEAR MY NEIGHBORS!" << std::endl;
        for (unsigned i = 0; i < neighbors.size(); i++)
          std::cout << "  MyNodeLoop neighbor i=" << i
                    << " => type: " << neighbors[i].getType() << std::endl;
        neighbors.clear();
      }
      std::cout << "~MyNodeLoop(" << val << "): FINISHED!" << std::endl;
    }
  }
  //
  friend ostream& operator<<(ostream& os, const MyNodeLoop& node) {
    os << "MyNodeLoop(" << node.val << ")";
    return os;
  }
};
*/