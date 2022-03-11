#pragma once

#include "utils.hpp"

using std::ostream;

// ==================
//  Node and List
// ==================
// Tree
// memory is self-managed

template <typename T>
class TNode {
  //
public:
  struct VarTNode {
    // TODO: variant
    sptr<TNode> node;
    wptr<TNode> _node;

    bool operator==(const sptr<TNode> other) const
    {
      if (node != other)
        return false;
      return other == _node.lock();
    }

    /*
    void do_reset()
    {
      node.reset();
      _node.reset();
    }
    */
  };

  using TNodeType = VarTNode;

private:
  //
  //vector<VarTNode> children;
  //
  //T value;
  //
public:
  //
  T value;
  //
  TNode(T value, wptr<TNode<T>> _root = wptr<TNode<T>>())
      : value { value }
      , tree_root { _root }
  {
  }

  ~TNode()
  {
    std::cout << "~TNode(" << value << ")" << std::endl;
  }

  // weak pointer back to root of tree (for root node, this field is empty/.reset())
  wptr<TNode<T>> tree_root;

  // recursive update on all owned nodes
  void recupdate(wptr<TNode<T>> new_tree_root)
  {
    //
    for (unsigned i = 0; i < children.size(); i++) {
      if (children[i].node) {
        // ???
        assert(this->tree_root.lock() == children[i].node->tree_root.lock());
        //
        children[i].node->tree_root = new_tree_root;
        children[i].node->recupdate(new_tree_root);
      } else {
        assert(false);
      }
    }
    // finally, update my own tree root ref
    this->tree_root = new_tree_root;
  }

  // weak pointer back to parent (NOT necessary for now)
  //wptr<TNode<T>> parent;

  // strong or weak pointer in children (mostly strong)
  vector<VarTNode> children;

  // get_child for traversal (maybe should remove this, to prevent ext. leakage)
  sptr<TNode> get_child(int i)
  {
    return children[i].node ? children[i].node : children[i]._node.lock();
  }

  auto add(sptr<TNode> nxt)
  {
    children.push_back(VarTNode { .node = nxt });
    children.at(children.size() - 1)._node.reset();
  }

  auto add_weak(wptr<TNode> nxt)
  {
    children.push_back(VarTNode { ._node = nxt });
    children.at(children.size() - 1).node.reset();
  }

  auto set_child(int i, sptr<TNode> nxt)
  {
    children[i].node = nxt;
    children[i]._node.reset();
  }
  //
  auto set_child_weak(int i, wptr<TNode> nxt)
  {
    children[i].node.reset();
    children[i]._node = nxt;
  }

  auto get_value() { return value; }

  friend ostream& operator<<(ostream& os, const TNode& me)
  {
    os << "TNode(" << me.value << ")";
    return os;
  }
};

template <typename T>
struct Tree {
  //
  sptr<TNode<T>> root; // owned reference
  //
  //wptr<TNode<T>> tail_node; // DAG behavior, but... non-owning

  Tree()
  {
    this->root = nullptr;
    //this->tail_node.reset();
  }

  void set_root(sptr<TNode<T>> _root)
  {
    this->root = _root;
    _root->tree_root = _root; // self-reference
  }

  // I Think... THIS WILL EXTEND the lifecycle of Data T, beyond limits of Tree container
  sptr<T> get_root_data()
  {
    // See: https://www.youtube.com/watch?v=JfmTagWcqoE
    // => using aliasing constructor
    // "Point to That and manage with This"
    // managing lifetime by construction
    return { root, &(root->value) };
  }

  ~Tree()
  {
    std::cout << "~Tree() => ";
    if (!this->root)
      std::cout << "nullptr" << std::endl;
    else
      std::cout << *this->root << std::endl;

    // prevents stackoverflow on recursive destructor...
    //while (!empty())
    //  pop_front();
  }

  bool empty() { return !root; }

  // add_child at node_ptr
  void add_child(sptr<TNode<T>> node_ptr, T v)
  {
    // case n=0: initialize root
    if (this->root == nullptr) {
      assert(node_ptr == nullptr);
      this->root = sptr<TNode<T>>(new TNode<T> { v });
      //this->tail_node = this->head;
      //this->tail_node.lock()->set_next_weak(this->head); // circular
      return;
    }
    // case n>=1: general
    auto node = sptr<TNode<T>>(new TNode<T> { v });
    node_ptr.add(node);
    //this->tail_node.lock()->set_next_weak(this->head); // circular
  }

  void print()
  {
    std::cout << "Tree::print() => root (exists=" << (bool)this->root << ")" << std::endl;
    std::cout << "{";
    if (this->root)
      printFrom(this->root);
    std::cout << "}";
    std::cout << std::endl;
  }

  void printFrom(sptr<TNode<T>> node)
  {
    if (node) {
      std::cout << "node=" << *node << std::endl;

      for (unsigned i = 0; i < node->children.size(); i++) {
        if (node->get_child(i) == this->root) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else
          printFrom(node->get_child(i));
      }
    }
  }
  /*
  //
  void push_back(T v)
  {
    // case n=0: initialize circular behavior (instead of nullptr)
    if (this->head == nullptr) {
      this->head = sptr<LNode<T>>(new LNode<T>(v, this->head));
      this->tail_node = this->head;
      this->tail_node.lock()->set_next_weak(this->head); // circular
      return;
    }
    // case n>=1: general
    auto node = sptr<LNode<T>>(new LNode<T>(v, this->head));
    node->set_next_weak(this->head); // circular
    this->tail_node.lock()->set_next(node); // chain
    // this->head = node;
    this->tail_node = node;
  }
  //
  T pop_front()
  {
    // assert: n > 0
    assert(this->head);
    // n == 1 (clean back to nullptr state)
    if (this->head == this->head->get_next()) {
      T v = this->head->get_value();
      this->head = nullptr;
      this->tail_node.reset();
      return v;
    }
    // n > 1
    T v = this->head->get_value();
    this->head = std::move(this->head->get_next());
    return v;
  }
  //
  void print()
  {
    sptr<LNode<T>> p = head;
    int i = 0;
    std::cout << "list (empty=" << empty() << ") ";
    // check emptyness
    while (p) {
      i++;
      std::cout << p->get_value() << " ";
      p = p->get_next();
      // check circularity
      if (p == head)
        break;
    }
    std::cout << std::endl;
  }
  */
};