#pragma once

#include <string>
#include <vector>

#include <cycles/List.hpp>

using std::vector, std::string;

struct Vertice {
  string label; // what else? // TODO: becomes general type T
};

struct Edge {
  double weight; // what else? // TODO: becomes general type T
};

struct Graph {
  std::vector<Vertice> vertex;
  std::vector<std::vector<Edge>> edges;

  // who is the owner?
  //
  // a "cycle" is a "meta-owner" of parts?
  //
  // a vertex is the owner of vertex? of edges?
  //
  // a graph is the owner of vertex? of edges? (This looks bad.. like sub-heap
  // or Arena... not looking for this)
  //
  // MAYBE, a vertex is the owner of other vertex, THROUGH a cyclic-list
  // thus preventing cycles in the "ownership chain"

  void print()
  {
    std::cout << " ====> WILL PRINT GRAPH" << std::endl;
    std::cout << "Graph (|V|=" << vertex.size() << "):" << std::endl;
    for (unsigned i = 0; i < vertex.size(); i++) {
      std::cout << "v: " << i << " -> " << vertex[i].label << std::endl;
    }
  }
};