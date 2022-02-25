#pragma once

#include "Tree.hpp"
#include "utils.hpp"

#include <vector>

using std::vector, std::ostream, std::map;

// ========================
// cycle_ptr and cycle_ctx
// ========================
// smart pointer suitable for cycles
// memory is self-managed
//-------------------------

template <typename T>
class cycle_ctx {
public:
  // collect strategy parameters
  bool auto_collect { true };

  // Forest management system comes here
  //using NodeType = sptr<TNode<T>>;
  //using TreeType = sptr<Tree<T>>;
  //
  using NodeType = sptr<TNode<sptr<T>>>;
  using TreeType = sptr<Tree<sptr<T>>>;

  // Forest: every Tree is identified by its Root node in map system
  map<NodeType, TreeType> forest;

public:
  cycle_ctx()
  {
    std::cout << "cycle_ctx created!" << std::endl;
  }

  ~cycle_ctx()
  {
    std::cout << "~cycle_ctx() forest_size =" << forest.size() << std::endl;
    for (auto p : forest) {
      std::cout << " clearing root of ~> " << p.first << "'" << (*p.first) << "' -> " << p.second << " TREE" << std::endl;
      p.second->root = nullptr; // clear root
    }
    forest.clear(); // is it necessary??
  }

  void collect()
  {
    assert(this);
    // force collect
    std::cout << "collect! NOT Implemented (this=" << this << ")" << std::endl;
  }

  void print()
  {
    std::cout << "print ctx: (forest size=" << forest.size() << ")" << std::endl;
    for (auto p : forest) {
      std::cout << " ~> " << p.first << "'" << (*p.first) << "' -> " << p.second << " TREE" << std::endl;
    }
  }
};

template <typename T>
class cycle_ptr {
private:
  wptr<cycle_ctx<T>> ctx;
  //
  // TODO: should be an element/vertex on tree
  sptr<T> ref; // TODO: remove this direct ref!
  //
  wptr<TNode<sptr<T>>> remote_node;

public:
  cycle_ptr(wptr<cycle_ctx<T>> ctx, T* t)
      : ctx { ctx }
      , ref { t }
  {
    std::cout << "creating NEW cycle_ptr (this_new=" << this << " t=" << t << ") ";
    if (ref)
      std::cout << "with ref -> " << *ref << std::endl;
    else
      std::cout << "with ref -> nullptr" << std::endl;
    //
    if (!ref)
      return; // SHOULD NOT CREATE A NEW TREE!
    //
    std::cout << "=> TODO Registering this in new Tree context!" << std::endl;
    auto node_new = sptr<TNode<sptr<T>>>(new TNode<sptr<T>> { ref });
    remote_node = node_new;
    //
    auto stree = sptr<Tree<sptr<T>>>(new Tree<sptr<T>> {});
    std::cout << "tree ~> ";
    stree->print();
    stree->set_root(node_new);
    ctx.lock()->forest[node_new] = stree;
    ctx.lock()->print();
    //stree.root = sptr<TNode<T>>(new TNode<T>(val, stree));
    //forest[ref] = stree;
  }

  cycle_ptr(wptr<cycle_ctx<T>> ctx, T* t, cycle_ptr<T>& owner)
      : ctx { ctx }
      , ref { t }
  {
    std::cout << "creating NEW OWNED cycle_ptr (this_new=" << this << " t=" << t << ") "
              << " owner='" << owner.get() << "' ";
    if (ref)
      std::cout << "with ref -> " << *ref << std::endl;
    else
      std::cout << "with ref -> nullptr" << std::endl;
    //
    if (!ref)
      return; // SHOULD NOT CREATE A NEW TREE OR A NEW RELATION
    //
    std::cout << "=> TODO Registering this in EXISTING Tree context!" << std::endl;
    //
    auto node_new = sptr<TNode<sptr<T>>>(new TNode<sptr<T>> { ref });
    remote_node = node_new;
    //
    // TREE OF OWNER MUST EXIST
    wptr<TNode<sptr<T>>> root_node = owner.remote_node.lock()->tree_root;
    sptr<TNode<sptr<T>>> lock_root_node = root_node.lock();
    if (!lock_root_node) {
      std::cout << "ERROR! cannot get root node" << std::endl;
      assert(false);
    }
    // FIND TREE
    auto tree_it = ctx.lock()->forest.find(lock_root_node);
    if (tree_it == ctx.lock()->forest.end()) {
      std::cout << "ERROR! could not find Tree!" << std::endl;
      assert(false);
    } else {
      std::cout << " ~~~> FOUND TREE" << std::endl;
      auto stree = tree_it->second;
      // setup root of new node as tree root
      node_new->tree_root = lock_root_node;
      // add dependency from 'owner'
      owner.remote_node.lock()->add(node_new);
      //
      std::cout << "tree ~> ";
      stree->print();
      ctx.lock()->print();
    }
    //
  }

  // copy constructor (still good for vector... must be the meaning of a "copy")
  cycle_ptr(const cycle_ptr<T>& copy, const cycle_ptr<T>& owner)
      : ctx { copy.ctx }
      , ref { copy.ref }
      , remote_node { copy.remote_node }
  {
    // guarantee that context is the same
    //assert(copy.ctx == owner.ctx);
    //
    std::cout << "TODO: Must register relation of (this_new="
              << this << ") '" << (this->get())
              << "' owned_by (this_other="
              << &owner << ") '" << (owner.get()) << "'"
              << std::endl;
    //
    // TREE OF OWNER MUST EXIST
    wptr<TNode<sptr<T>>> root_node = owner.remote_node.lock()->tree_root;
    sptr<TNode<sptr<T>>> lock_root_node = root_node.lock();
    if (!lock_root_node) {
      std::cout << "ERROR2! cannot get root node" << std::endl;
      assert(false);
    }
    // FIND TREE
    auto tree_it = ctx.lock()->forest.find(lock_root_node);
    if (tree_it == ctx.lock()->forest.end()) {
      std::cout << "ERROR2! could not find Tree!" << std::endl;
      assert(false);
    } else {
      std::cout << " ~~~> FOUND TREE" << std::endl;
      auto stree = tree_it->second;
      //
      // check that this does not have tree, or belong to same tree
      auto my_tree_root = this->remote_node.lock()->tree_root.lock();
      if (!my_tree_root) {
        std::cout << "THIS NODE HAS NO ROOT!" << std::endl;
        assert(false);
      }
      auto this_tree_it = ctx.lock()->forest.find(my_tree_root);
      if (this_tree_it == ctx.lock()->forest.end()) {
        std::cout << "ERROR2: THIS NODE (" << this->remote_node.lock() << ") HAS NO TREE" << std::endl;
        std::cout << "Tree root -> " << my_tree_root << std::endl;
        this->ctx.lock()->print();
        assert(false);
      }
      auto this_tree = this_tree_it->second;
      if (this_tree == stree) {
        std::cout << " ~~ SUCCESS! Tree is the same! MUST ADD Weak" << std::endl;
        owner.remote_node.lock()->add_weak(this->remote_node);
      } else {
        std::cout << " ~~ ERROR2! Tree is NOT the same!" << std::endl;
        std::cout << "this_tree = " << this_tree << std::endl;
        std::cout << "stree = " << stree << std::endl;
        assert(false);
      }
      //
      std::cout << "tree ~> ";
      stree->print();
      ctx.lock()->print();
    }
    //
  }

  // returns a self-copy and setup ownership relationship to 'owner'
  //
  // important: (this is an idea)
  // - if you copy pointer again (with regular copy constructor),
  // ownership relationship will be kept on ctx
  // example: b = a.copy_owned(c); // b is a copy of a, and relationship c->b is created (so as c->a)
  // - maybe this is a good thing, because we can keep copy constructor
  // - maybe not, but I don't imagine why at this moment...
  auto copy_owned(const cycle_ptr<T>& owner)
  {
    return cycle_ptr<T>(*this, owner);
  }

  // maybe we need an 'owns' method, that does the opposite
  // example: a.owns(b);  will create relationship a->b
  void owns(const cycle_ptr<T>& owned)
  {
    assert(false);
  }

  // maybe we need an 'removed_owned' method
  // MAYBE NOT! maybe this is just a ctx thing, or maybe we just let variables expire to handle this...
  // I don't really know.
  // example: a.remove_owned(b);  will remove relationship a->b (if it exists, it returns true)
  // what to do with opposite relationship? do we let ctx handle all this?
  bool remove_owned(const cycle_ptr<T>& owned)
  {
    assert(false);
  }

  /*
  cycle_ptr(const cycle_ptr<T>& other)
      : ctx { other.ctx }
      , ref { other.ref }
  {
    std::cout << "copy cycle_ptr" << std::endl;
    std::cout << "Must register relation of (this) " << this << (this->get()) << " weakly_owns-> " << &other << (other.get()) << std::endl;
  }
  */

  //~cycle_ptr()
  //{
  //do_reset();
  //}

  /*
  void do_reset()
  {
    ref.reset();
    ctx.reset();
  }
*/

  auto get_ctx() -> wptr<cycle_ctx<T>>
  {
    return ctx;
  }

  bool operator==(const cycle_ptr<T>& other) const
  {
    // do not comparing null pointers as 'true' (why?)... just feels like right now. (thinking more of refs than pointers)
    //(this->has_get() && other.has_get()) &&
    // TODO: think more.
    return (this->has_get() == other.has_get()) && (ctx.lock() == other.ctx.lock()) && (ref == other.ref);
  }

  bool has_get() const
  {
    return (bool)ref;
  }

  T& get()
  {
    return *ref;
  }

  const T& get() const
  {
    return *ref;
  }

  /*
  // TODO: avoid this!
  T* operator->()
  {
    return ref.get();
  }
  */
};

// ================== EXAMPLE ================

template <typename X>
class MyNode {
public:
  X val;

  vector<cycle_ptr<MyNode>> neighbors;
  //
  friend ostream& operator<<(ostream& os, const MyNode& node)
  {
    os << "MyNode(" << node.val << ")";
    return os;
  }
};

// ---------

template <typename X>
class MyGraph {
  using MyNodeX = MyNode<X>;

private:
  sptr<cycle_ctx<MyNodeX>> ctx;

public:
  auto my_ctx() -> wptr<cycle_ctx<MyNodeX>>
  {
    return this->ctx;
  }

  auto make_node(X v) -> cycle_ptr<MyNodeX>
  {
    return cycle_ptr<MyNodeX>(this->ctx, new MyNodeX { .val = v });
  }

  auto make_node_owned(X v, cycle_ptr<MyNodeX>& owner) -> cycle_ptr<MyNodeX>
  {
    return cycle_ptr<MyNodeX>(this->ctx, new MyNodeX { .val = v }, owner);
  }

  auto make_null_node() -> cycle_ptr<MyNodeX>
  {
    return cycle_ptr<MyNodeX>(this->ctx, nullptr);
  }

  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  cycle_ptr<MyNodeX> entry;

  MyGraph()
      : entry { make_null_node() }
      , ctx { new cycle_ctx<MyNodeX> {} }
  {
  }

  ~MyGraph()
  {
    std::cout << "~MyGraph" << std::endl;
    ctx = nullptr;
    //entry.do_reset();
  }

  void print()
  {
    std::cout << "MyGraph::print() => root.has_get() ~ " << entry.has_get() << std::endl;
    std::cout << "MyGraph::ctx -> " << ctx << std::endl;
    if (ctx)
      ctx->print();
    if (entry.has_get())
      printFrom(entry);
    std::cout << std::endl;
  }

  void printFrom(cycle_ptr<MyNodeX> node)
  {
    if (node.has_get()) {
      std::cout << "node=" << node.get() << std::endl;

      for (unsigned i = 0; i < node.get().neighbors.size(); i++) {
        if (node.get().neighbors[i] == entry) {
          std::cout << "WARNING: cyclic graph! stop printing..." << std::endl;
        } else
          printFrom(node.get().neighbors[i]);
      }
    }
  }
  //
};