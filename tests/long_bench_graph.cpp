
// C++
#include <chrono>
#include <functional>
#include <set>
#include <string>
#include <vector>
//
#include <cycles/relation_ptr.hpp>
//
#include "TestGraph.hpp"
//
// ================== EXAMPLE ================

using namespace cycles;       // NOLINT
using namespace std::chrono;  // NOLINT

// generate experiment to replicate on all graph types
std::vector<std::vector<int>> gen_experiment(int v, int e, int seed,
                                             bool multigraph) {
  srand(seed);
  std::vector<std::vector<int>> v_index(v);
  for (int c = 0; c < e; c++) {
    int i = ::rand() % v;
    int j = ::rand() % v;
    // loops are allowed with i==j
    v_index[i].push_back(j);
  }
  // remove edge duplicates (or leave as multigraph)
  if (!multigraph) {
    for (int i = 0; i < v; i++) {
      std::sort(v_index[i].begin(), v_index[i].end());
      auto last = std::unique(v_index[i].begin(), v_index[i].end());
      v_index[i].erase(last, v_index[i].end());
    }
  }
  //
  return v_index;
}

// print experiment graph
void print_exp(const std::vector<std::vector<int>>& exp) {
  std::cout << "print_exp" << std::endl;
  std::cout << "N=" << exp.size() << std::endl;
  for (unsigned i = 0; i < exp.size(); i++) {
    std::cout << "i=" << i << " |edges|=" << exp[i].size() << ": ";
    for (unsigned e = 0; e < exp[i].size(); e++) {
      std::cout << exp[i][e] << " ";
    }
    std::cout << std::endl;
  }
}

// ===========================================

namespace rust_example1 {

sptr<Node> init1(int v, const std::vector<std::vector<int>>& v_index) {
  std::vector<sptr<Node>> vertex;
  for (int i = 0; i < v; i++)
    vertex.push_back(sptr<Node>{new Node(std::to_string(i))});
  // make mirror experiment with v_index
  for (int i = 0; i < v; i++) {
    for (int e = 0; e < v_index[i].size(); e++) {
      int j = v_index[i][e];
      vertex[i]->edges.push_back(vertex[j]);
    }
  }
  // root is first vertex only... the rest may die automatically. let's see.
  return vertex[0];
}

void test_main1(int V, int E, int SEED) {
  std::vector<std::vector<int>> v_index = gen_experiment(V, E, SEED, false);
  //
  auto g = init1(V, v_index);
  const auto& gref = *g;
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  sptr<Node> f = gref.first();
  foo(*f);
}

}  // namespace rust_example1

namespace rust_example2 {

Node* init1(int v, const std::vector<std::vector<int>>& v_index,
            TypedArena<Node>& arena)  // NOLINT
{                                     // NOLINT
  std::vector<Node*> vertex;
  for (int i = 0; i < v; i++) {
    std::string stri = std::to_string(i);
    // NOLINTNEXTLINE
    auto* node = new Node(stri, arena);
    vertex.push_back(node);
  }
  // make mirror experiment with v_index
  for (int i = 0; i < v; i++) {
    for (int e = 0; e < v_index[i].size(); e++) {
      int j = v_index[i][e];
      vertex[i]->edges.push_back(vertex[j]);
    }
  }
  // root is first vertex only... the rest may die automatically. let's see.
  return vertex[0];
}

void test_main1(int V, int E, int SEED) {
  //
  std::vector<std::vector<int>> v_index = gen_experiment(V, E, SEED, false);
  //
  auto arena = TypedArena<Node>{};
  auto g = init1(V, v_index, arena);
  const auto& gref = *g;
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  Node* f = gref.first();
  foo(*f);
}

}  // namespace rust_example2

namespace cycles_example1 {

std::pair<relation_pool, relation_ptr<Node>> init_long_rptr(
    int v, const std::vector<std::vector<int>>& v_index) {
  //
  relation_pool pool;
  std::vector<relation_ptr<Node>> vertex;
  for (int i = 0; i < v; i++) {
    std::string stri = std::to_string(i);
    // NOLINTNEXTLINE
    auto* node = new Node(stri);
    vertex.push_back(relation_ptr<Node>{pool.getContext(), node});
  }
  // make mirror experiment with v_index
  for (int i = 0; i < v; i++) {
    for (int e = 0; e < v_index[i].size(); e++) {
      int j = v_index[i][e];
      vertex[i]->edges.push_back(vertex[j].copy_owned(vertex[i]));
    }
  }
  //
  // pool.getContext()->debug = true;
  // root is first vertex only... the rest may die automatically. let's see.
  return std::pair<relation_pool, relation_ptr<Node>>{std::move(pool),
                                                      std::move(vertex[0])};
}

void test_main_long_rptr(int V, int E, int SEED) {
  std::vector<std::vector<int>> v_index = gen_experiment(V, E, SEED, false);
  // DEBUG
  // print_exp(v_index);
  //
  auto gpair = init_long_rptr(V, v_index);
  // std::cout << "root = " << gpair.second->datum << std::endl;
  const auto& gref = *(gpair.second.get());
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  if (gref.edges.size() > 0) {
    Node* f = gref.first();
    foo(*f);
  } else {
    std::cout << "WARNING: no edges to invoke foo()" << std::endl;
  }
}

}  // namespace cycles_example1

namespace cycles_example2_arena {

relation_ptr<Node> init_long_rptr(int v,
                                  const std::vector<std::vector<int>>& v_index,
                                  TypedArenaCycles<Node>& arena)  // NOLINT
{                                                                 // NOLINT
  //
  std::vector<relation_ptr<Node>> vertex;
  for (int i = 0; i < v; i++) {
    std::string stri = std::to_string(i);
    // NOLINTNEXTLINE
    auto* node = new Node(stri);
    vertex.push_back(arena.alloc(node));
  }
  // make mirror experiment with v_index
  for (int i = 0; i < v; i++) {
    for (int e = 0; e < v_index[i].size(); e++) {
      int j = v_index[i][e];
      vertex[i]->edges.push_back(vertex[j].copy_owned(vertex[i]));
    }
  }
  //
  // pool.getContext()->debug = true;
  // root is first vertex only... the rest may die automatically. let's see.
  return std::move(vertex[0]);
}

void test_main_long_rptr(int V, int E, int SEED) {
  std::vector<std::vector<int>> v_index = gen_experiment(V, E, SEED, false);
  // DEBUG
  // print_exp(v_index);
  //
  auto arena = TypedArenaCycles<Node>{};
  auto ptr = init_long_rptr(V, v_index, arena);
  // std::cout << "root = " << gpair.second->datum << std::endl;
  const auto& gref = *(ptr.get());
  std::set<std::string> seen;
  gref.traverse([&](const std::string& d) { std::cout << d; }, seen);
  if (gref.edges.size() > 0) {
    Node* f = gref.first();
    foo(*f);
  } else {
    std::cout << "WARNING: no edges to invoke foo()" << std::endl;
  }
}

}  // namespace cycles_example2_arena

int main() {
  int V = 500;
  int E = (int)(V * V * 0.6);  // NOLINT
  int SEED = 999999;
  std::cout << "V=" << V << " E=" << E << std::endl;
  std::cout << "sptr_example1 (leaks due to cycle in sptr)" << std::endl;
  auto c = high_resolution_clock::now();
  {
    // many things...
    rust_example1::test_main1(V, E, SEED);
  }
  // will leak, surely.
  std::cout
      << "example1 "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =====================================

  std::cout << "arena_example2 (no leaks due to arena)" << std::endl;
  c = high_resolution_clock::now();
  {
    // many things...
    rust_example2::test_main1(V, E, SEED);
  }
  // will not leak
  std::cout
      << "example2 "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =====================================

  std::cout
      << "cycles_example3 (no leaks expected (hopefully!) due to relation_ptr)"
      << std::endl;
  c = high_resolution_clock::now();
  {
    // many things...
    cycles_example1::test_main_long_rptr(V, E, SEED);
  }
  // will not leak
  std::cout
      << "cycles example3 "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =====================================

  std::cout << "cycles_example4 with arena (no leaks expected (hopefully!) due "
               "to relation_ptr)"
            << std::endl;
  c = high_resolution_clock::now();
  {
    // many things...
    cycles_example2_arena::test_main_long_rptr(V, E, SEED);
  }
  // will not leak
  std::cout
      << "cycles example4 "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  // =====================================

  std::cout << std::endl;
  std::cout << "SHOULD LEAK ONLY ON: rust_example1::test_main1()" << std::endl;
  std::cout << "FINISHED!" << std::endl;

  return 0;
}
