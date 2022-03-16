#pragma once

#include "utils.hpp"

using std::ostream;

// ============================
//   memory-managed Tree Node
// ============================
// Tree Node
// all memory is self-managed

static int tnode_count = 0;

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
  TNode(T value, wptr<TNode<T>> _parent = wptr<TNode<T>>())
      : value { value }
      , parent { _parent }
  //, tree_root { _root }
  {
    tnode_count++;
    std::cout << "TNode tnode_count = " << tnode_count << std::endl;
  }

  ~TNode()
  {
    std::cout << "~TNode(" << value << ")" << std::endl;
    tnode_count--;
    std::cout << "  -> ~TNode tnode_count = " << tnode_count << std::endl;
  }

  // weak pointer back to root of tree (for root node, this field is empty/.reset())
  //wptr<TNode<T>> tree_root;
  //
  wptr<TNode<T>> parent;

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

  // IMPORTANT! This method would work better with a std::set or std::map on children
  bool has_child(wptr<TNode> target)
  {
    auto t_sptr = target.lock();
    if (!t_sptr) {
      assert(false); // STRANGE...
      return false;
    }
    for (unsigned i = 0; i < children.size(); i++) {
      if (children[i].node) {
        if (t_sptr == children[i].node)
          return true;
      } else {
        if (t_sptr == children[i]._node.lock())
          return true;
      }
    }
    return false;
  }

  auto add_child_strong(sptr<TNode> nxt)
  {
    // check if parent is set correctly
    assert(this == nxt->parent.lock().get());
    //
    children.push_back(VarTNode { .node = nxt });
    children.at(children.size() - 1)._node.reset();
  }

  auto add_child_weak(wptr<TNode> nxt)
  {
    // check if parent is set correctly
    //assert(this == nxt.lock()->parent.lock().get());
    std::cout << "IMPORTANT! add_child_weak does not require parent to be REAL parent..."
                 "it could be problem! solution: use two lists (for weak or strong)"
              << std::endl;
    //
    children.push_back(VarTNode { ._node = nxt });
    children.at(children.size() - 1).node.reset();
  }

  // false if not found, OR target is nullptr
  bool remove_child(TNode* target)
  {
    if (!target)
      return false;
    for (unsigned i = 0; i < children.size(); i++) {
      if (children[i].node) {
        if (target == children[i].node.get()) {
          children.erase(children.begin() + i);
          return true;
        }
      } else {
        if (target == children[i]._node.lock().get()) {
          children.erase(children.begin() + i);
          return true;
        }
      }
    }
    return false;
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
