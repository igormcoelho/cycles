
#include <map>
//
#include <cycles/List.hpp>
#include <cycles/Tree.hpp>
#include <cycles/cycles_ptr.hpp>
#include <cycles/nodes_exp.hpp>
#include <cycles/utils.hpp>
#include <demo_cptr/Graph.hpp>

using std::string, std::vector, std::map;

int main() {
  //
  // demos for cyclic List types
  //
  {
    List<double> l;
    l.print();
    auto node1 = sptr<LNode<double>>(new LNode<double>(1, l.head));
    l.head = node1;
    auto node2 = sptr<LNode<double>>(new LNode<double>(2, l.head));
    l.head = node2;
    auto node3 = sptr<LNode<double>>(new LNode<double>(3, l.head));
    l.head = node3;

    // node1->set_next(node3); // loop and leak
    node1->set_next_weak(node3);  // loop but no leak

    l.print();
  }

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

  {
    List<double> l;
    l.print();
    l.push_back(20);
    l.push_back(21);
    l.print();
  }

  {
    List<double> l;
    l.print();
    l.push_front(9);
    l.push_front(10);
    l.push_front(11);
    l.push_front(12);
    l.print();
    std::cout << "will push_back" << std::endl;
    l.push_back(13);
    l.push_back(14);
    l.print();
    while (!l.empty()) {
      l.pop_front();
      l.print();
    }
  }
  {
    List<double> l;
    std::cout << "will create big list!" << std::endl;
    for (unsigned i = 0; i < 30000; i++) l.push_front(0.0);
    std::cout << "will free big list!" << std::endl;
    // Stackoverflow on Destructor with N=30000
    /*
    std::cout << "manual free to prevent stackoverflow" << std::endl;
    while (!l.empty())
      l.pop_front();
    l.print();
    */
  }

  std::cout << "FINISHED!" << std::endl;
  return 0;
}