// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

// c++
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
// lib
#include <cycles/cycles_ptr.hpp>

using namespace cycles;  // NOLINT
using namespace std;     // NOLINT

class Counter {
 public:
  Counter() { ++count_; }
  ~Counter() { --count_; }
  static int count() { return count_; }

 private:
  static int count_;
};

int Counter::count_ = 0;

/*--- Solution 1 (default) ----------------------------------------------------

class MyGraph {
public:
        class Node : public Counter {
                vector<shared_ptr<Node>> children;
        public:
                void AddChild(const shared_ptr<Node>& node) {
                        children.push_back(node);
                }
                void RemoveChild(const shared_ptr<Node>& node) {
                        auto it = find(children.begin(), children.end(), node);
                        Expects(it != children.end() && "trying to remove a
child that was never added"); children.erase(it);
                }
        };

        void SetRoot(const shared_ptr<Node>& node) {
                root = node;
        }

        void ShrinkToFit() {
        }

        static auto MakeNode() { return make_shared<MyGraph::Node>(); }

private:
        shared_ptr<Node> root;
};

//--- Solution 2 (deferred_ptr) -----------------------------------------------
/*/

// static deferred_heap heap;

class MyGraphGCPP {
 public:
  class Node : public Counter {
    vector<cycles_ptr<Node>> children;  // {heap};

    char c;

   public:
    explicit Node(char _c) : c{_c} {}

    void AddChild(const cycles_ptr<Node>& node) { children.push_back(node); }

    void RemoveChild(const cycles_ptr<Node>& node) {
      auto it = find(children.begin(), children.end(), node);
      assert(it != children.end());  // NOLINT
      // children.erase(it);
      // *it = nullptr;
      *it = make_null_node_helper(node.get_ctx());
    }

    auto make_null_node_helper(auto ctx) -> cycles_ptr<Node> {
      return cycles_ptr<Node>(ctx, nullptr);
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& me) {
      os << "Node(" << me.c << ")";
      return os;
    }
  };

  void SetRoot(const cycles_ptr<Node>& node) { root = node; }

  // void ShrinkToFit() { heap.collect(); }
  void ShrinkToFit() {}  // TODO(igormcoelho): allow collection here ???

  // static auto MakeNode() { return heap.make<MyGraph::Node>(); }

  auto MakeNode(char c) -> cycles_ptr<Node> {
    auto* ptr = new Node(c);  // NOLINT
    // TODO(igormcoelho): create ctx->make<...> and type erase ctx
    return cycles_ptr<Node>(this->ctx, ptr);
  }

  // TODO(igormcoelho): remove this helper for null node
  auto make_null_node() -> cycles_ptr<Node> {
    return cycles_ptr<Node>(this->ctx, nullptr);
  }
  // TODO(igormcoelho): can't this be defaulted somehow?
  MyGraphGCPP() : ctx{new cycles_ctx<Node>{}}, root{make_null_node()} {}

 public:
  sptr<cycles_ctx<Node>> ctx;

 private:
  cycles_ptr<Node> root;
};

// ----------------------------------------------------------------------------
//*/

bool TestCase1() {
  std::cout << "Initialize graph" << std::endl;
  MyGraphGCPP g;
  std::cout << std::endl << "TestCase1: MyGraph created!" << std::endl;
  {
    // auto a = MyGraph::MakeNode();
    std::cout << std::endl << "A" << std::endl;
    auto a = g.MakeNode('a');
    std::cout << std::endl << "B" << std::endl;
    g.SetRoot(a);
    std::cout << std::endl << "C" << std::endl;
    auto b = g.MakeNode('b');
    std::cout << std::endl << "D" << std::endl;
    a->AddChild(b.copy_owned(a));
    std::cout << std::endl << "E" << std::endl;
    auto c = g.MakeNode('c');
    std::cout << std::endl << "F" << std::endl;
    b->AddChild(c.copy_owned(b));
    std::cout << std::endl << "G" << std::endl;
    a->RemoveChild(b);
    std::cout << std::endl << "H" << std::endl;
  }
  g.ShrinkToFit();
  std::cout << "forest size: " << g.ctx->forest.size() << std::endl;
  if (Counter::count() == 1) {
    return true;
  } else {
    std::cout << "Counter::count = " << Counter::count() << " (should be 1)"
              << std::endl;
    return false;
  }
}

/*
bool TestCase2() {
  MyGraph g;
  {
    // auto a = MyGraph::MakeNode();
    auto a = g.MakeNode();
    g.SetRoot(a);
    auto b = g.MakeNode();
    a->AddChild(b);
    auto c = g.MakeNode();
    b->AddChild(c);
    auto d = g.MakeNode();
    b->AddChild(d);
    d->AddChild(b);
    a->RemoveChild(b);
  }
  g.ShrinkToFit();
  if (Counter::count() == 1) {
    return true;
  } else {
    std::cout << "Counter::count = " << Counter::count() << " (should be 1)"
              << std::endl;
    return false;
  }
}

bool TestCase3() {
  MyGraph g;
  {
    // auto a = MyGraph::MakeNode();
    auto a = g.MakeNode();
    g.SetRoot(a);
    auto b = g.MakeNode();
    a->AddChild(b);
    auto c = g.MakeNode();
    b->AddChild(c);
    auto d = g.MakeNode();
    b->AddChild(d);
    d->AddChild(b);
  }
  g.ShrinkToFit();
  if (Counter::count() == 4) {
    return true;
  } else {
    std::cout << "Counter::count = " << Counter::count() << " (should be 4)"
              << std::endl;
    return false;
  }
}

bool TestCase4() {
  MyGraph g;
  {
    // auto a = MyGraph::MakeNode();
    auto a = g.MakeNode();
    g.SetRoot(a);
    auto b = g.MakeNode();
    a->AddChild(b);
    auto c = g.MakeNode();
    b->AddChild(c);
    auto d = g.MakeNode();
    b->AddChild(d);
    d->AddChild(b);
    d->RemoveChild(b);
  }
  g.ShrinkToFit();
  if (Counter::count() == 4) {
    return true;
  } else {
    std::cout << "Counter::count = " << Counter::count() << " (should be 1)"
              << std::endl;
    return false;
  }
}
*/

int main() {
  cout.setf(ios::boolalpha);

  bool passed1 = TestCase1();
  cout << "test1:" << passed1 << endl;
  if (!passed1) return -1;

  /*
    bool passed2 = TestCase2();
    cout << "test2:" << passed2 << endl;
    if (!passed2) return -1;

    bool passed3 = TestCase3();
    cout << "test3:" << passed3 << endl;
    if (!passed3) return -1;

    bool passed4 = TestCase4();
    cout << "test4:" << passed4 << endl;
    if (!passed4) return -1;
  */

  return 0;
}
