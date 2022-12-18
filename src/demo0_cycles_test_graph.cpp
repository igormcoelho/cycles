// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

// c++
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
// lib
#include <cycles/cycle_ptr.hpp>

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

class MyGraph {
 public:
  class Node : public Counter {
    vector<cycle_ptr<Node>> children;  // {heap};

   public:
    void AddChild(const cycle_ptr<Node>& node) { children.push_back(node); }

    void RemoveChild(const cycle_ptr<Node>& node) {
      auto it = find(children.begin(), children.end(), node);
      assert(it != children.end());  // NOLINT
      // children.erase(it);
      // *it = nullptr;
      *it = make_null_node_helper(node.get_ctx());
    }

    auto make_null_node_helper(auto ctx) -> cycle_ptr<Node> {
      return cycle_ptr<Node>(ctx, nullptr);
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& me) {
      os << "Node(...)";
      return os;
    }
  };

  void SetRoot(const cycle_ptr<Node>& node) { root = node; }

  // void ShrinkToFit() { heap.collect(); }
  void ShrinkToFit() {}  // TODO(igormcoelho): allow collection here ???

  // static auto MakeNode() { return heap.make<MyGraph::Node>(); }

  auto MakeNode() -> cycle_ptr<Node> {
    auto* ptr = new Node();  // NOLINT
    // TODO(igormcoelho): create ctx->make<...> and type erase ctx
    return cycle_ptr<Node>(this->ctx, ptr);
  }

  // TODO(igormcoelho): remove this helper for null node
  auto make_null_node() -> cycle_ptr<Node> {
    return cycle_ptr<Node>(this->ctx, nullptr);
  }
  // TODO(igormcoelho): can't this be defaulted somehow?
  MyGraph() : root{make_null_node()}, ctx{new cycle_ctx<Node>{}} {}

 private:
  cycle_ptr<Node> root;
  sptr<cycle_ctx<Node>> ctx;
};

// ----------------------------------------------------------------------------
//*/

bool TestCase1() {
  MyGraph g;
  std::cout << std::endl << "TestCase1: MyGraph created!" << std::endl;
  {
    // auto a = MyGraph::MakeNode();
    auto a = g.MakeNode();
    g.SetRoot(a);
    auto b = g.MakeNode();
    a->AddChild(b);
    auto c = g.MakeNode();
    b->AddChild(c);
    a->RemoveChild(b);
  }
  g.ShrinkToFit();
  return Counter::count() == 1;
}

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
  return Counter::count() == 1;
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
  return Counter::count() == 4;
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
  return Counter::count() == 4;
}

int main() {
  cout.setf(ios::boolalpha);

  bool passed1 = TestCase1();
  cout << passed1 << endl;

  bool passed2 = TestCase2();
  cout << passed2 << endl;

  bool passed3 = TestCase3();
  cout << passed3 << endl;

  bool passed4 = TestCase4();
  cout << passed4 << endl;

  return 0;
}