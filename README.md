# cycles
Smart pointers in C++ for cyclic data structures.

This project proposes a new kind of smart pointer called `relation_ptr`, that represents a "pointer to relations between objects". These objects belong to a common "pool", called `relation_pool`.
These structures have cycle-breaking properties, thus allowing usage when `std::shared_ptr` alone fails to clean memory.

**Note:** *This is inspired by [hsutter/gcpp](https://github.com/hsutter/gcpp) project, `relation_pool` works like `deferred_heap`, and `relation_ptr` works like `deferred_ptr` (without the need of custom allocators or manual invocation of `collect()` method).*

**General recommendation:** as with gcpp project, this smart pointer type should be used only when `std::unique_ptr` and `std::shared_ptr` (with `std::weak_ptr`) do not work, specially with cyclic structures. So, this is expected to be innefficient, but it is meant to be easy to use and leak-free.


## Motivation: implementing a Graph

Consider node structure:

```{.cpp}
using cycles::relation_ptr;
using cycles::relation_pool;

class MyNode {
public:
  double val;
  std::vector<relation_ptr<MyNode>> neighbors;
};


class MyGraph {
public:
  relation_pool pool;
  
  // Example: graph with entry, similar to a root in trees... but may be cyclic.
  relation_ptr<MyNode> entry;

  auto make_node(double v) -> relation_ptr<MyNode>
  {
    return relation_ptr<MyNode>(pool.getContext(), new MyNode { .val = v });
  }

  MyGraph()
      : entry { relation_ptr<MyNode>{} }
  {
  }

  ~MyGraph()
  {
    // optional, no need to clean anything
    pool.clear();
  }

};
```

This example shows that, even for a cyclic graph, no leaks happen!
Graph stores a `relation_pool` while all `relation_ptr` ensure that no real cycle dependencies exist. The `relation_ptr` has move-only semantics, but it is possible to
create new relations pointing to the same objects by using helper method `copy_owned`.

```{.cpp}
  {
    MyGraph<double> G;
    G.pool.getContext()->print();
    //
    G.entry = G.make_node(-1.0);
    // make cycle
    relation_ptr<MyNode> ptr1 = G.make_node(1.0);
    g.entry->neighbors.push_back(ptr1.copy_owned(G.entry));
    relation_ptr<MyNode> ptr2 = G.make_node(2.0);
    ptr1->neighbors.push_back(ptr2.copy_owned(ptr1));
    relation_ptr<MyNode> ptr3 = G.make_node(3.0);
    ptr2->neighbors.push_back(ptr3.copy_owned(ptr1));
    // finish cycle
    ptr3->neighbors.push_back(G.entry.copy_owned(ptr3));
  }
  // This will not leak! (even if a cycle exists)
```

## features

This pointer has advantages (mostly inspired by gcpp project):

- **header only project** (just copy the `.hpp` and it works)
- **zero overhead C++17 structure** (costs are only applied to who uses this project, and proportional to the number of smart pointer allocations)
- **deferred destruction** (such as gcpp)
- type erased pool type (or context, or arena, or section, etc) that automatically discards all references in same pool (such as gcpp)
- ability to manually destroy/collect garbage on convenient locations (such as gcpp)
- deferred destruction by means of stored destructors and not raw memory (such as gcpp)
- **automatic destruction of objects, regardless of cycles** (not available in gcpp)
- **relatively simple implementation aiming cross-language support, based on tree ownership data structures** (gcpp is implemented with a completely different strategy focused on C/C++, not yet considered for other languages, as far as I know)

This pointer has disadvantages too:

- **experimental project, likely with hidden bugs and inneficient implementation** (SERIOUS testing and benchmarking considered! ALL existing tests are passing!)
- slower performance, compared to `std::shared_ptr` (see benchmarks below)
- slower performance, compared to `gcpp` (supposed... must validate, yet)
- likely thread unsafe (must investigate deeper)
- does not support copy semantics, only move semantics and a `copy_owned` method as helper
- (planned feature) no support for delegated construction of smart pointer (such as in `std::shared_ptr` two parameter constructor)

## Typical use cases

- developing cyclic data structures using an unified pointer type. *See [ExperimentsList.md](ExperimentsList.md) to learn more about that.*
- developing simple data structures, such as lists, where `std::unique_ptr` fails with native recursive destruction (due to stack-overflow). *See [ExperimentsList.md](ExperimentsList.md).*
- developing graph-based data structures, without worrying about memory cleanups, or performing manual memory cleanups (specially where efficiency is not top priority)
- prototyping in a simpler way without memory-leaks, and later improving the code with native `std::unique_ptr`, `std::shared_ptr` and `std::weak_ptr`
- prototyping with a clear strategy of **ownership** and **relations** between entities, completely isolated in memory pools

## Tests and benchmark

`make test`

### some benchmarks

`make bench`

#### benchmark of construction compared with `std::shared_ptr`

```
relation_ptr: 11396.6ms
shared_ptr: 1968.57ms
```

Around 5x slower just to construct 10 million smart pointers.

#### benchmark of deferred destruction of list and tree

Considering 100k elements:

```
UList unique_ptr: 4.62166ms
SList shared_ptr: 9.85481ms
CList relation_ptr: 68.9792ms (15x uptr, 7x sptr)
CList no auto_collect relation_ptr: 126.484ms
```

- Around 13x slower than `std::unique_ptr`
- Around 7x slower than `std::shared_ptr`
- Worse behavior disabling automatic garbage collection


Considering 10M elements (`std::unique_ptr` destructor adapted to prevent stack overflow):
```
UList unique_ptr: 406.094ms
SList shared_ptr: 576.623ms
CList relation_ptr: 5702.24ms (14x uptr, 10x sptr)
CList no auto_collect relation_ptr: 9502.88ms
```

- Around 14x slower than `std::unique_ptr`
- Around 10x slower than `std::shared_ptr`
- Worse behavior disabling automatic garbage collection

Tests with trees with 2^15 ~ 32k elements
```
UTree with unique_ptr: 1.9145ms
STree with shared_ptr: 4.38139ms
CTree with relation_ptr: 285.272ms (150x uptr, 65x sptr)
CTree with relation_ptr - no auto_collect: 251.257ms (132x uptr, 57x sptr)
```

- Around 57x slower over `std::shared_ptr`
- Better behavior when garbage is not collected during operations (only in the end)

#### benchmarks against Arena strategies

Considering graph with 500 vertex and 150k edges:

```
V=500 E=150000
sptr_example1 (leaks due to cycle in sptr)
foo: 2
example1 13.4712ms
arena_example2 (no leaks due to arena)
foo: 2
example2 10.7775ms
cycles_example3 (no leaks expected (hopefully!) due to relation_ptr)
foo: 2
cycles example3 1184.28ms
```

`relation_ptr` is around 120x slower than pure arena strategy (with vector of unique_ptr).

It means that Arena strategies should be used whenever possible... specially, when
block allocation is feasible and no partial collection is needed during execution.

## How this works

This is implemented using efficient tree ownership data structures.

More details will come soon, regarding the expected operations of the underlying types, so as more efficient alternatives to implement this proposed smart pointer type.

## Interesting Projects

This project can be used to manage cyclic data structures with memory safe.

See https://github.com/hsutter/gcpp and its video from cppcon 2016 [Leak Freedom in C++... by Default](https://www.youtube.com/watch?v=JfmTagWcqoE).

## Acknowledgements

We appreciate the interest of all involved in this project.
Special thanks to Wang Yong Qiang for early revisions of the ideas behind this project.

## License

MIT License

Copyleft 2022-2023, Igor Machado Coelho
