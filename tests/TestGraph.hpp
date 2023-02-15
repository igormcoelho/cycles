#pragma once

// C++
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
//
#include <cycles/relation_ptr.hpp>
//
#include <hsutter-gcpp/deferred_allocator.h>
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

namespace rust_example1 {
// RC graph
// FROM: https://github.com/nrc/r4cppp/blob/master/graphs/src/rc_graph.rs

struct Node {
  std::string datum;
  std::vector<sptr<Node>> edges;

  explicit Node(const std::string& datum) : datum{datum}, edges{} {}

  void traverse(std::function<void(const std::string&)> f,
                std::set<std::string>& seen) const {
    //
    if (seen.find(this->datum) == seen.end()) {
      return;
    }
    f(this->datum);
    seen.insert(this->datum);
    for (auto& n : this->edges) {
      n->traverse(f, seen);
    }
  }

  sptr<Node> first() const { return this->edges[0]; }
};

void foo(const Node& node) { std::cout << "foo: " << node.datum << std::endl; }

sptr<Node> init() {
  auto root = sptr<Node>{new Node("A")};
  auto b = sptr<Node>{new Node("B")};
  auto c = sptr<Node>{new Node("C")};
  auto d = sptr<Node>{new Node("D")};
  auto e = sptr<Node>{new Node("E")};
  auto f = sptr<Node>{new Node("F")};

  {
    auto& mut_root = *root;
    mut_root.edges.push_back(b);
    mut_root.edges.push_back(c);
    mut_root.edges.push_back(d);

    auto& mut_c = *c;
    mut_c.edges.push_back(e);
    mut_c.edges.push_back(f);
    mut_c.edges.push_back(root);
  }

  return root;
}

void test_main() {
  auto g = init();
  const auto& gref = *g;
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  sptr<Node> f = gref.first();
  foo(*f);
}

}  // namespace rust_example1

namespace rust_example2 {
// REF graph ARENA
// https://github.com/nrc/r4cppp/blob/master/graphs/src/ref_graph.rs

template <class T>
struct TypedArena {
  std::vector<std::unique_ptr<T>> v;
  void alloc(T* t) { v.push_back(std::unique_ptr<T>{t}); }
};

struct Node {
  std::string datum;
  std::vector<Node*> edges;

  // NOLINTNEXTLINE
  Node(const std::string& datum, TypedArena<Node>& arena) : datum{datum} {
    arena.alloc(this);
  }

  void traverse(std::function<void(const std::string&)> f,
                std::set<std::string>& seen) const  // NOLINT
  {                                                 // NOLINT
    if (seen.find(this->datum) == seen.end()) {
      return;
    }
    f(this->datum);
    seen.insert(this->datum);
    for (auto* n : this->edges) {
      n->traverse(f, seen);
    }
  }

  Node* first() const { return this->edges[0]; }
};

void foo(const Node& node) { std::cout << "foo: " << node.datum << std::endl; }

// NOLINTNEXTLINE
Node* init(TypedArena<Node>& arena) {
  auto* root = new Node("A", arena);
  auto* b = new Node("B", arena);
  auto* c = new Node("C", arena);
  auto* d = new Node("D", arena);
  auto* e = new Node("E", arena);
  auto* f = new Node("F", arena);

  root->edges.push_back(b);
  root->edges.push_back(c);
  root->edges.push_back(d);

  c->edges.push_back(e);
  c->edges.push_back(f);
  c->edges.push_back(root);

  return root;
}

void test_main() {
  auto arena = TypedArena<Node>{};
  Node* g = init(arena);
  std::set<std::string> seen;
  g->traverse([&](const std::string& d) { std::cout << d; }, seen);
  foo(*g->first());
}

}  // namespace rust_example2

namespace cycles_example1 {
// RC graph
// Similar to rust_example1

struct Node {
  std::string datum;
  std::vector<relation_ptr<Node>> edges;

  explicit Node(const std::string& datum) : datum{datum}, edges{} {}

  void traverse(std::function<void(const std::string&)> f,
                std::set<std::string>& seen) const {
    //
    if (seen.find(this->datum) == seen.end()) {
      return;
    }
    f(this->datum);
    seen.insert(this->datum);
    for (auto& n : this->edges) {
      n->traverse(f, seen);
    }
  }

  // TODO(igormcoelho): this part could be improved with delegated sptr
  Node* first() const { return this->edges.at(0).get(); }

  friend std::ostream& operator<<(std::ostream& os, const Node& me) {
    os << "Node(\"" << me.datum << "\")";
    return os;
  }
};

void foo(const Node& node) { std::cout << "foo: " << node.datum << std::endl; }

std::pair<relation_pool<>, relation_ptr<Node>> init() {
  relation_pool<> pool;
  auto root = relation_ptr<Node>{new Node("A"), pool};
  auto b = relation_ptr<Node>{new Node("B"), pool};
  auto c = relation_ptr<Node>{new Node("C"), pool};
  auto d = relation_ptr<Node>{new Node("D"), pool};
  auto e = relation_ptr<Node>{new Node("E"), pool};
  auto f = relation_ptr<Node>{new Node("F"), pool};

  {
    auto& mut_root = *(root.get());
    mut_root.edges.push_back(b.get_owned(root));
    mut_root.edges.push_back(c.get_owned(root));
    mut_root.edges.push_back(d.get_owned(root));

    auto& mut_c = *(c.get());
    mut_c.edges.push_back(e.get_owned(c));
    mut_c.edges.push_back(f.get_owned(c));
    mut_c.edges.push_back(root.get_owned(c));
  }

  return std::pair<relation_pool<>, relation_ptr<Node>>{std::move(pool),
                                                        std::move(root)};
}

void test_main() {
  auto gpair = init();
  const auto& gref = *(gpair.second.get());
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  Node* f = gref.first();
  foo(*f);
}

}  // namespace cycles_example1

namespace cycles_example2_arena {
// REF graph ARENA

template <class T>
struct TypedArenaCycles {
  relation_pool<> pool;
  std::vector<cycles::relation_ptr<T>> v;
  //
  relation_ptr<T> alloc(T* t) {
    cycles::relation_ptr<T> ptr{t, pool};
    // observer pointer pattern for relation_ptr
    cycles::relation_ptr<T> ob_ptr = ptr.get_owned(ptr);
    v.push_back(std::move(ptr));
    return std::move(ob_ptr);
  }
};

struct Node {
  std::string datum;
  std::vector<relation_ptr<Node>> edges;

  explicit Node(const std::string& datum) : datum{datum}, edges{} {}

  void traverse(std::function<void(const std::string&)> f,
                std::set<std::string>& seen) const {
    //
    if (seen.find(this->datum) == seen.end()) {
      return;
    }
    f(this->datum);
    seen.insert(this->datum);
    for (auto& n : this->edges) {
      n->traverse(f, seen);
    }
  }

  // TODO(igormcoelho): this part could be improved with delegated sptr
  Node* first() const { return this->edges.at(0).get(); }

  friend std::ostream& operator<<(std::ostream& os, const Node& me) {
    os << "Node(\"" << me.datum << "\")";
    return os;
  }
};

void foo(const Node& node) { std::cout << "foo: " << node.datum << std::endl; }

relation_ptr<Node> init(TypedArenaCycles<Node>& arena)  // NOLINT
{                                                       // NOLINT
  auto root = arena.alloc(new Node("A"));               // NOLINT
  auto b = arena.alloc(new Node("B"));                  // NOLINT
  auto c = arena.alloc(new Node("C"));                  // NOLINT
  auto d = arena.alloc(new Node("D"));                  // NOLINT
  auto e = arena.alloc(new Node("E"));                  // NOLINT
  auto f = arena.alloc(new Node("F"));                  // NOLINT

  {
    auto& mut_root = *(root.get());
    mut_root.edges.push_back(b.get_owned(root));
    mut_root.edges.push_back(c.get_owned(root));
    mut_root.edges.push_back(d.get_owned(root));

    auto& mut_c = *(c.get());
    mut_c.edges.push_back(e.get_owned(c));
    mut_c.edges.push_back(f.get_owned(c));
    mut_c.edges.push_back(root.get_owned(c));
  }

  return std::move(root);
}

void test_main() {
  auto arena = TypedArenaCycles<Node>{};
  auto ptr = init(arena);
  const auto& gref = *(ptr.get());
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  Node* f = gref.first();
  foo(*f);
}

}  // namespace cycles_example2_arena

namespace gcpp_example1 {
// RC graph
// Similar to cycles_example1, using hsutter gcpp

using namespace gcpp;  // NOLINT

struct Node {
  std::string datum;
  deferred_vector<deferred_ptr<Node>> edges;

  explicit Node(const std::string& datum, deferred_heap& h)
      : datum{datum}, edges{h} {}

  void traverse(std::function<void(const std::string&)> f,
                std::set<std::string>& seen) const {
    //
    if (seen.find(this->datum) == seen.end()) {
      return;
    }
    f(this->datum);
    seen.insert(this->datum);
    // error: invalid conversion from ‘const void*’ to ‘void*’
    // strange error: set(get() + offset)
    // for (auto& n : this->edges) {
    for (unsigned i = 0; i < edges.size(); i++) {
      auto& n = edges[i];
      n->traverse(f, seen);
    }
  }

  // TODO(igormcoelho): this part could be improved with delegated sptr
  Node* first() const { return this->edges.at(0).get(); }

  friend std::ostream& operator<<(std::ostream& os, const Node& me) {
    os << "Node(\"" << me.datum << "\")";
    return os;
  }
};

void foo(const Node& node) { std::cout << "foo: " << node.datum << std::endl; }

deferred_ptr<Node> init(deferred_heap& my_heap) {
  auto root = my_heap.make<Node>("A", my_heap);
  auto b = my_heap.make<Node>("B", my_heap);
  auto c = my_heap.make<Node>("C", my_heap);
  auto d = my_heap.make<Node>("D", my_heap);
  auto e = my_heap.make<Node>("E", my_heap);
  auto f = my_heap.make<Node>("F", my_heap);

  {
    auto& mut_root = *(root.get());
    mut_root.edges.push_back(b);
    mut_root.edges.push_back(c);
    mut_root.edges.push_back(d);

    auto& mut_c = *(c.get());
    mut_c.edges.push_back(e);
    mut_c.edges.push_back(f);
    mut_c.edges.push_back(root);
  }

  return root;
}

void test_main() {
  deferred_heap my_heap;
  auto root = init(my_heap);
  const auto& gref = *(root.get());
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  Node* f = gref.first();
  foo(*f);
}

}  // namespace gcpp_example1
