# cycles
Data Structures in C++ with Cycles

The cyclic List works as expected (for teaching purposes), but can be considered experimental (for production).

This is alpha software, still in early phases.

## Initial Testing (alpha)

`bazel build ...`

`valgrind ./bazel-bin/demo0_cycles_test`

## 'cyclic' List

The first structure tested here is a cyclic List, made by `std::shared_ptr` and `std::weak_ptr`.

This List can be extended to cases where `std::unique_ptr` ("strong") and raw ptr ("weak") are used.

### Current Methods

- `push_front(T)`
- `pop_front() -> T`
- `push_back(T)`
- `empty() -> bool`

There's currently no `pop_back()`, as tail_node link cannot be updated backwards.

Maybe, we could also support some sort of circular Double Linked List, to complement this forward / singly linked list.

### Example

```{.cpp}
  {
    List<double> l;
    l.print();
    l.push_front(5);
    l.push_front(6);
    l.push_front(7);
    l.print();
    std::cout << "will push_back" << std::endl;
    l.push_back(8);
    l.push_back(9);
    l.print();
  }
```

Memory is cleaned up automatically.

```
make
valgrind ./app_demo 
```

No leaks are ever expected.

## Advanced Example with Graph

Consider node structure:

```
template <typename X>
class MyNode {
public:
  X val;
  vector<cycles_ptr<MyNode>> neighbors;
};

template <typename X>
class MyGraph {
  using MyNodeX = MyNode<X>;

private:
  sptr<cycles_ctx> ctx;

public:
  auto my_ctx() -> wptr<cycles_ctx>
  {
    return this->ctx;
  }

  auto make_node(X v) -> cycles_ptr<MyNodeX>
  {
    return cycles_ptr<MyNodeX>(this->ctx, new MyNodeX { .val = v });
  }

  auto make_node_owned(X v, cycles_ptr<MyNodeX>& owner) -> cycles_ptr<MyNodeX>
  {
    return cycles_ptr<MyNodeX>(this->ctx, new MyNodeX { .val = v }, owner);
  }

  auto make_null_node() -> cycles_ptr<MyNodeX>
  {
    return cycles_ptr<MyNodeX>(this->ctx, nullptr);
  }

  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  cycles_ptr<MyNodeX> entry;

  MyGraph()
      : entry { make_null_node() }
      , ctx { new cycles_ctx {} }
  {
  }

  ~MyGraph()
  {
    ctx = nullptr;
  }

};
```

This DRAFT example shows that, even for a cyclic graph, no leaks happen!
Graph stores a cycle_ctx while all cycles_ptr ensure that no real cycle dependencies exist.

```
  {
    MyGraph<double> G;
    G.my_ctx().lock()->print();
    //
    G.entry = G.make_node(-1.0);
    G.print();
    // make cycle
    using MyNodeX = MyNode<double>;
    cycles_ptr<MyNodeX> ptr1 = G.make_node_owned(1.0, G.entry);
    cycles_ptr<MyNodeX> ptr2 = G.make_node_owned(2.0, ptr1);
    cycles_ptr<MyNodeX> ptr3 = G.make_node_owned(3.0, ptr2);
    // JUST ASSIGN OWNED TO head again... copy is not really necessary (TODO: create other method)
    auto ptr_head = G.entry.copy_owned(ptr3);
  }
  // This will not leak! (even if a cycle exists)
```

Currently, we need to handle Tree merging, as expected, in order to deal with other examples.

TODO:
- Out of order insertions that create multiple Trees (requiring merges)
- Removal of arcs (much harder), but with forseen strategies
   * make bridge and no realtime remove (could be handled with callbacks, to allow realtime usage)
   * immediate restructuring of trees and weak links (likely requires running through trees)
      - we can try to improve this too! but at the moment, priority is to make it work

### Improvements

The typical recursive behavior of destructors can limit the usage up to stack limit.
For this reason, the implemented destructor prevents such poor behavior (so List can grow bigger than stack limit).

However, we plan to add more transparent support for the internals of list, although quite experimental.

## Other experiments

This is quite experimental, so other structures are expected to join these soon.

## Interesting Projects

This project can be used to manage cyclic data structures with memory safe.

See https://github.com/hsutter/gcpp and its video from cppcon 2016

## License

Free to use

dual LGPLv3 and MIT License

Copyleft 2022, Igor Machado Coelho
