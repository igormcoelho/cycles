#pragma once

#include <cycles/detail/utils.hpp>

// ==================
//  Node and List
// ==================
// Singly Linked List or Forward List
// memory is self-managed

template <typename T>
class LNode {
 private:
  //
  T value;
  //
  // TODO(igormcoelho): variant
  sptr<LNode> next;
  wptr<LNode> _next;
  //
 public:
  LNode(T value, sptr<LNode> next) : value{value}, next{next} {}

  // get_next for traversal (maybe should remove this, to prevent ext. leakage)
  sptr<LNode> get_next() { return next ? next : _next.lock(); }

  auto set_next(sptr<LNode> nxt) {
    next = nxt;
    _next.reset();
  }
  //
  auto set_next_weak(wptr<LNode> nxt) {
    next.reset();
    _next = nxt;
  }

  auto get_value() { return value; }
};

template <typename T>
struct List {
  //
  sptr<LNode<T>> head;  // owned reference
  //
  wptr<LNode<T>> tail_node;  // DAG behavior, but... non-owning

  List() {
    this->head = nullptr;
    this->tail_node.reset();
  }

  ~List() {
    // prevents stackoverflow on recursive destructor...
    while (!empty()) pop_front();
  }

  bool empty() { return !head; }

  //
  void push_front(T v) {
    // case n=0: initialize circular behavior (instead of nullptr)
    if (this->head == nullptr) {
      this->head = sptr<LNode<T>>(new LNode<T>(v, this->head));
      this->tail_node = this->head;
      this->tail_node.lock()->set_next_weak(this->head);  // circular
      return;
    }
    // case n>=1: general
    auto node = sptr<LNode<T>>(new LNode<T>(v, this->head));
    this->head = node;
    this->tail_node.lock()->set_next_weak(this->head);  // circular
  }
  //
  void push_back(T v) {
    // case n=0: initialize circular behavior (instead of nullptr)
    if (this->head == nullptr) {
      this->head = sptr<LNode<T>>(new LNode<T>(v, this->head));
      this->tail_node = this->head;
      this->tail_node.lock()->set_next_weak(this->head);  // circular
      return;
    }
    // case n>=1: general
    auto node = sptr<LNode<T>>(new LNode<T>(v, this->head));
    node->set_next_weak(this->head);         // circular
    this->tail_node.lock()->set_next(node);  // chain
    // this->head = node;
    this->tail_node = node;
  }
  //
  T pop_front() {
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
  void print() {
    sptr<LNode<T>> p = head;
    int i = 0;
    std::cout << "list (empty=" << empty() << ") ";
    // check emptyness
    while (p) {
      i++;
      std::cout << p->get_value() << " ";
      p = p->get_next();
      // check circularity
      if (p == head) break;
    }
    std::cout << std::endl;
  }
};