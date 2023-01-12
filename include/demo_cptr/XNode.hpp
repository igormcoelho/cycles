#pragma once

// C++
#include <map>
#include <vector>
//
#include <cycles/cycles_ptr.hpp>
#include <cycles/detail/Tree.hpp>
#include <cycles/detail/utils.hpp>
#include <pre-experiments/List.hpp>
#include <pre-experiments/nodes_exp.hpp>
//
#include <demo_cptr/Graph.hpp>

using std::string, std::vector, std::map;  // NOLINT

struct XNode {
  int content;
  vector<sptr<XNode>> nodes;

  explicit XNode(int _content) : content{_content} {}

  friend std::ostream& operator<<(std::ostream& os, const XNode& node) {
    os << "XNode(" << node.content << ")";
    return os;
  }
};

template <typename TNode>
struct XEdge {
  TNode to;
  // has_next: DOES THIS XEdge also provides a step to next? Example, return to
  // HEAD? (in case of last in list) example: HEAD=1   to=6   has_next=true, so,
  // an implicit arc from 6 to 1 exists.
  //
  // note this does not have 'from', so it's implicit where this edge actually
  // came from, from cyclic List
  // TODO: maybe we need 'from' here... we need to analyse if this helps or
  // not.. or even make it harder to deal with.
  //
  bool has_next = false;
  //
  XEdge(TNode _to, bool _has_next = false) : to{_to}, has_next{_has_next} {}
  //
  friend std::ostream& operator<<(std::ostream& os, const XEdge& e) {
    os << "[to=" << e.to << "; " << e.has_next << "]";
    return os;
  }
};