
#include <cycles/relation_ptr.hpp>

using std::string, std::vector, std::map;

using cycles::relation_pool;
using cycles::relation_ptr;

class MyNode {
 public:
  double val;

  explicit MyNode(double _val) : val{_val} {}

 private:
  relation_ptr<MyNode> self;  // self-ownership pattern
  relation_ptr<MyNode> _next;
  relation_ptr<MyNode> _prev;

 public:
  void set_self(const relation_ptr<MyNode>& node) {
    if (!self && (node.get() == this)) self = node.get_owned(node);
  }

  relation_ptr<MyNode> next() const { return _next.get_owned(_next); }
  relation_ptr<MyNode> prev() const { return _prev.get_owned(_prev); }

  void set_next(const relation_ptr<MyNode>& node) {
    self->_next = node.get_owned(self);
  }

  void set_prev(const relation_ptr<MyNode>& node) {
    self->_prev = node.get_owned(self);
  }

  // add node as self->next, and node->prev as self
  void add_next(const relation_ptr<MyNode>& node) {
    assert(node);
    assert(self);
    self->_next = node.get_owned(self);
    node->_prev = self.get_owned(node);
  }
};

class MyList {
 private:
  // no pool needed
  relation_ptr<MyNode> _entry;

 public:
  relation_ptr<MyNode> entry() { return _entry.get_owned(_entry); }

  void set_entry(double val) {
    sptr<cycles::detail::DynowForestV1> ctx{
        new cycles::detail::DynowForestV1{}};
    _entry = relation_ptr<MyNode>{new MyNode{val}, ctx};
    _entry->set_self(_entry);
  }

  // helper function to generate new pointers according to same 'pool'
  auto make_self_node(double val) -> relation_ptr<MyNode> {
    assert(_entry);
    relation_ptr<MyNode> node{new MyNode{val}, _entry.get_ctx()};
    node->set_self(node);  // self-ownership pattern
    return node;
  }
};

void printFrom(relation_ptr<MyNode> current, const relation_ptr<MyNode>& origin,
               bool isFirst = false) {
  if (!current || !origin) return;
  if (isFirst || (current.get() != origin.get())) {
    std::cout << current.get()->val << " ";
    if (current->next()) printFrom(current->next(), origin);
  }
}

int main() {
  // begin example3
  {
    MyList L;

    // create nodes -1, 1, 2 and 3
    L.set_entry(-1.0);
    assert(L.entry());
    L.entry()->add_next(L.make_self_node(1));
    L.entry()->next()->add_next(L.make_self_node(2));
    L.entry()->next()->next()->add_next(L.make_self_node(3));
    // manually finish cycle
    L.entry()->next()->next()->next()->set_next(L.entry());
    L.entry()->set_prev(L.entry()->next()->next()->next());

    //
    std::cout << "printFrom(" << L.entry()->val << ")" << std::endl;
    printFrom(L.entry(), L.entry(), true);
    std::cout << "finished" << std::endl;
  }
  // This will not leak! Even when a cycle exists from 'L.entry'

  return 0;
}
