// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

// c++
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
// lib
#include <cycles/detail/v1/DynowForestV1.hpp>
#include <cycles/relation_pool.hpp>
#include <cycles/relation_ptr.hpp>

using namespace cycles;  // NOLINT
using namespace std;     // NOLINT

// FROM: https://github.com/hsutter/gcpp

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

static relation_pool<> pool;

class MyGraph {
 public:
  class Node : public Counter {
    vector<relation_ptr<Node>> children;

   public:
    // child node must be owned before arriving here.
    void AddChild(relation_ptr<Node> node) {
      children.push_back(std::move(node));
    }
    void RemoveChild(const relation_ptr<Node>& node) {
      auto it = find(children.begin(), children.end(), node);
      // Expects(it != children.end() &&
      //         "trying to remove a child that was never added");
      //  children.erase(it);
      //
      //  *it = nullptr;
      it->reset();
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& me) {
      os << "Node()";
      return os;
    }
  };

  // GetRoot changed from const& to copy/move by igormcoelho
  void SetRoot(relation_ptr<Node> node) { root = std::move(node); }

  // GetRoot added by igormcoelho
  relation_ptr<Node>& GetRoot() { return root; }

  void ShrinkToFit() { pool.getContext()->collect(); }

  static auto MakeNode() {
    return relation_ptr<Node>::make(::pool.getContext());
  }

 private:
  relation_ptr<Node> root;
};

// ----------------------------------------------------------------------------
//*/

bool TestCase1() {
  MyGraph g;
  {
    // auto a = ;
    g.SetRoot(MyGraph::MakeNode());
    auto b = MyGraph::MakeNode();
    g.GetRoot()->AddChild(b.get_owned(g.GetRoot()));
    auto c = MyGraph::MakeNode();
    b->AddChild(c.get_owned(b));
    g.GetRoot()->RemoveChild(b);
  }
  g.ShrinkToFit();
  return Counter::count() == 1;
}

bool TestCase2() {
  MyGraph g;
  {
    auto a = MyGraph::MakeNode();
    g.SetRoot(std::move(a));
    auto b = MyGraph::MakeNode();
    g.GetRoot()->AddChild(b.get_owned(g.GetRoot()));
    auto c = MyGraph::MakeNode();
    b->AddChild(c.get_owned(b));
    auto d = MyGraph::MakeNode();
    b->AddChild(d.get_owned(b));
    d->AddChild(b.get_owned(d));
    g.GetRoot()->RemoveChild(b);
  }
  g.ShrinkToFit();
  return Counter::count() == 1;
}

bool TestCase3() {
  MyGraph g;
  {
    // auto a = ;
    g.SetRoot(MyGraph::MakeNode());
    auto b = MyGraph::MakeNode();
    g.GetRoot()->AddChild(b.get_owned(g.GetRoot()));
    auto c = MyGraph::MakeNode();
    b->AddChild(c.get_owned(b));
    auto d = MyGraph::MakeNode();
    b->AddChild(d.get_owned(b));
    d->AddChild(b.get_owned(d));
  }
  g.ShrinkToFit();
  return Counter::count() == 4;
}

bool TestCase4() {
  MyGraph g;
  {
    // auto a = MyGraph::MakeNode();
    g.SetRoot(MyGraph::MakeNode());
    auto b = MyGraph::MakeNode();
    g.GetRoot()->AddChild(b.get_owned(g.GetRoot()));
    auto c = MyGraph::MakeNode();
    b->AddChild(c.get_owned(b));
    auto d = MyGraph::MakeNode();
    b->AddChild(d.get_owned(b));
    d->AddChild(b.get_owned(d));
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
