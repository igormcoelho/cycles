
#include <cycles/relation_ptr.hpp>

using std::string, std::vector, std::map;

using cycles::relation_pool;
using cycles::relation_ptr;

class MyNode {
 public:
  relation_ptr<MyNode> self;  // self-ownership pattern

  double val;
  std::vector<relation_ptr<MyNode>> neighbors;

  explicit MyNode(double val_) : val{val_} {}

  void add_neighbor(const relation_ptr<MyNode>& node) {
    self->neighbors.push_back(node.get_owned(self));
  }
};

class MyGraph {
 public:
  // Example: graph with entry, similar to a root in trees... but may be cyclic
  relation_pool<> pool;        // pool of data, similar to 'deferred_heap'
  relation_ptr<MyNode> entry;  // pointer to data, similar to 'deferred_ptr'

  // helper function to generate new pointers according to same 'pool'
  auto make_self_node(double v) -> relation_ptr<MyNode> {
    relation_ptr<MyNode> node = pool.make<MyNode>(v);
    node->self = node.get_owned(node);  // self-ownership pattern
    return node;
  }
};

void printFrom(relation_ptr<MyNode> target, const relation_ptr<MyNode>& origin,
               bool isFirst = false) {
  if (!target || !origin) return;
  if (isFirst || (target.get() != origin.get())) {
    std::cout << target.get()->val << " ";

    for (const auto& node : target->neighbors)
      printFrom(node.get_self_owned(), origin);
  }
}

int main() {
  // begin example2
  {
    MyGraph G;

    // create nodes -1, 1, 2 and 3
    G.entry = G.make_self_node(-1.0);
    relation_ptr<MyNode> ptr1 = G.make_self_node(1.0);
    relation_ptr<MyNode> ptr2 = G.make_self_node(2.0);
    relation_ptr<MyNode> ptr3 = G.make_self_node(3.0);

    // manually generate a cycle: -1 -> 1 -> 2 -> 3 -> -1 -> ...
    // entry node -1 has neighbor node 1
    G.entry->add_neighbor(ptr1);
    // node 1 has neighbor node 2
    ptr1->add_neighbor(ptr2);
    // node 2 has neighbor node 3
    ptr2->add_neighbor(ptr3);
    // finish cycle: node 3 has neighbor entry node -1
    ptr3->add_neighbor(G.entry);

    // optional, destroy local variables (only keep 'G.entry')
    ptr1.reset();
    ptr2.reset();
    ptr3.reset();

    // nodes 1, 2, 3, -1, 1, 2, 3, -1, 1 ... still reachable from entry node
    // -1
    assert(-1 == G.entry->val);
    assert(2 == G.entry->neighbors[0]->neighbors[0]->val);
    assert(
        -1 ==
        G.entry->neighbors[0]->neighbors[0]->neighbors[0]->neighbors[0]->val);
    //
    std::cout << "printFrom(" << G.entry->val << ")" << std::endl;
    printFrom(G.entry.get_self_owned(), G.entry, true);
    std::cout << "finished" << std::endl;
  }
  // This will not leak! Even when a cycle exists from 'G.entry'

  return 0;
}
