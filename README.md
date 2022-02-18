# cycles
Data Structures in C++ with Cycles

The cyclic List works as expected (for teaching purposes), but can be considered experimental (for production).

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
