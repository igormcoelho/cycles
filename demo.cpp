
#include <map>

#include "Graph.hpp"
#include <cycles/List.hpp>
#include <cycles/Tree.hpp>
#include <cycles/cycle_ptr.hpp>
#include <cycles/nodes_exp.hpp>
#include <cycles/utils.hpp>

using std::string, std::vector, std::map;

struct XNode {
  int content;
  vector<sptr<XNode>> nodes;

  XNode(int _content)
      : content { _content }
  {
  }

  friend std::ostream& operator<<(std::ostream& os, const XNode& node)
  {
    os << "XNode(" << node.content << ")";
    return os;
  }
};

template <typename TNode>
struct XEdge {
  TNode to;
  // has_next: DOES THIS XEdge also provides a step to next? Example, return to HEAD? (in case of last in list)
  // example: HEAD=1   to=6   has_next=true, so, an implicit arc from 6 to 1 exists.
  //
  // note this does not have 'from', so it's implicit where this edge actually came from, from cyclic List
  // TODO: maybe we need 'from' here... we need to analyse if this helps or not.. or even make it harder to deal with.
  //
  bool has_next = false;
  //
  XEdge(TNode _to, bool _has_next = false)
      : to { _to }
      , has_next { _has_next }
  {
  }
  //
  friend std::ostream& operator<<(std::ostream& os, const XEdge& e)
  {
    os << "[to=" << e.to << "; " << e.has_next << "]";
    return os;
  }
};

int main()
{
  std::cout << "sizeof(List1) = " << sizeof(List1) << std::endl;
  std::cout << "sizeof(Node1) = " << sizeof(Node1) << std::endl;
  //
  std::cout << "sizeof(List<double>) = " << sizeof(List<double>) << std::endl;
  std::cout << "sizeof(Node<double>) = " << sizeof(LNode<double>) << std::endl;
  //
  std::cout << "sizeof(Node2_Variant) = " << sizeof(Node2_Variant) << std::endl;
  std::cout << "sizeof(Node3) = " << sizeof(Node3) << std::endl;
  std::cout << "sizeof(Node4_Herb) = " << sizeof(Node4_Herb) << std::endl;

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
    node1->set_next_weak(node3); // loop but no leak

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
    for (unsigned i = 0; i < 30000; i++)
      l.push_front(0.0);
    std::cout << "will free big list!" << std::endl;
    // Stackoverflow on Destructor with N=30000
    /*
    std::cout << "manual free to prevent stackoverflow" << std::endl;
    while (!l.empty())
      l.pop_front();
    l.print();
    */
  }
  {
    Graph g;
    g.vertex.push_back(Vertice { .label = "1" });
    g.vertex.push_back(Vertice { .label = "2" });
    //
    g.print();
    //

    // Is this a "CyclicArena"? An arena that has different lifetimes per cycles, not the whole arena?
    vector<sptr<XNode>> all_nodes;
    //
    // register new node (through some factory "CyclicArena" method, get_new_node()?)
    all_nodes.push_back(sptr<XNode> { new XNode { 0 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 1 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 2 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 3 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 4 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 5 } });
    all_nodes.push_back(sptr<XNode> { new XNode { 6 } });
    //
    // DISPOSALS will only possible when this "main" node ref is dropped...
    // it means that real-time systems can take good care of these references,
    // and let the "internal world explode", when it's convenient
    //
    vector<List<XEdge<sptr<XNode>>>> cycles;
    List<XEdge<sptr<XNode>>> empty_list;
    cycles.push_back(empty_list); // 0
    cycles.push_back(empty_list); // 1

    vector<int> ref_cycles;
    ref_cycles.push_back(-1); // 0
    ref_cycles.push_back(-1); // 1
    ref_cycles.push_back(-1); // 2

    //
    // create edge 1->2, where 1 is owner of 2
    //
    // does 1 have a cycle already? if not create one (HEAD to 1)
    cycles[1].push_back(all_nodes[1]);
    ref_cycles[1] = 1; // OWNER is 1
    // if FROM 1 to destination 2 not have cycle, just add 2 after position of node 1 in cycle 1
    cycles[1].push_back(all_nodes[2]);
    ref_cycles[2] = 1; // OWNER is 1
    //
    // create edge 2->3, where 2 is owner of 3
    //
    // does 2 have a cycle already? Resp.: 1. So follow it, and put 3 from position of node 2 in cycle 1
    // (just assuming last node is used here)
    cycles[1].push_back(all_nodes[3]);
    ref_cycles[3] = 1; // OWNER is 1
    //
    //
    // create edge 3->6, where 3 is owner of 6
    //
    // does 3 have a cycle already? Resp.: 1. So follow it, and put 6 from position of node 3 in cycle 1
    // (just assuming last node is used here)
    cycles[1].push_back(all_nodes[6]);

    //
    List<XEdge<XNode>> l_A;
    l_A.push_back(XNode { 1 });
    l_A.push_back(XNode { 2 });
    l_A.push_back(XNode { 3 });
    l_A.push_back(XEdge(XNode { 6 }, false));
    // l_A.push_front(XEdge(6, false));
    // l_A.push_front(3);
    // l_A.push_front(2);
    // l_A.push_front(1);
    std::cout << "X head to ";
    l_A.print();
    std::cout << "A: " << l_A.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_B;
    l_B.push_front(XNode { 6 });
    l_B.push_front(XNode { 13 });
    l_B.push_front(XNode { 12 });
    l_B.push_front(XNode { 8 });
    l_B.push_front(XNode { 7 });
    l_B.push_front(XNode { 10 });
    std::cout << "head to ";
    l_B.print();
    std::cout << "B: " << l_B.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_C;
    l_C.push_front(XEdge(XNode { 8 }, false));
    l_C.push_front(XNode { 11 });
    l_C.push_front(XNode { 7 });
    std::cout << "head to ";
    l_C.print();
    std::cout << "C: " << l_C.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_D;
    l_D.push_front(XEdge(XNode { 7 }, false));
    l_D.push_front(XNode { 3 });
    std::cout << "head to ";
    l_D.print();
    std::cout << "D: " << l_D.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_E;
    l_E.push_front(XEdge(XNode { 3 }, false));
    l_E.push_front(XNode { 4 });
    l_E.push_front(XNode { 8 });
    std::cout << "head to ";
    l_E.print();
    std::cout << "E: " << l_E.head.get() << std::endl;
    //
  }

  //=========
  //
  {
    std::cout << std::endl;
    std::cout << "========= " << std::endl;

    using NodeDouble = sptr<TNode<double>>; ///typename TNode<double>::TNodeType;

    map<NodeDouble, sptr<Tree<double>>> mp;

    auto node1 = NodeDouble(new TNode<double> { 2.0 });
    auto node2 = NodeDouble(new TNode<double> { 3.0 });

    auto t1 = sptr<Tree<double>>(new Tree<double> {});
    t1->root = node1;
    std::cout << "t1.root = " << *t1->root << std::endl;
    //Tree<double> t2;
    auto t2 = sptr<Tree<double>>(new Tree<double> {});
    t2->root = node2;
    std::cout << "t2.root = " << *t2->root << std::endl;

    mp[node1] = t1;
    mp[node2] = t2;

    mp[node1]->print();
    mp[node2]->print();

    sptr<double> data1 = mp[node1]->get_root_data();
    sptr<double> data2 = mp[node2]->get_root_data();
    std::cout << "data1 = " << (data1 ? *data1 : -9999) << std::endl;
    std::cout << "data2 = " << (data2 ? *data2 : -9999) << std::endl;
    std::cout << "DROP Tree => ";
    t1->print();
    //
    t1 = nullptr;
    mp[node1] = nullptr;
    std::cout << "data1 = " << (data1 ? *data1 : -9999) << std::endl;
    std::cout << "data2 = " << (data2 ? *data2 : -9999) << std::endl;
  }
  //
  {
    std::cout << std::endl;
    std::cout << "========= " << std::endl;

    using NodeDouble = sptr<TNode<sptr<double>>>;
    using TreeDouble = sptr<Tree<sptr<double>>>;

    map<NodeDouble, TreeDouble> mp;

    auto node1 = NodeDouble(new TNode<sptr<double>> { std::make_shared<double>(2.0) });
    auto node2 = NodeDouble(new TNode<sptr<double>> { std::make_shared<double>(3.0) });

    auto t1 = TreeDouble(new Tree<sptr<double>> {});
    t1->root = node1;
    std::cout << "t1.root = " << *t1->root->get_value() << std::endl;
    //Tree<double> t2;
    auto t2 = TreeDouble(new Tree<sptr<double>> {});
    t2->root = node2;
    std::cout << "t2.root = " << *t2->root->get_value() << std::endl;

    mp[node1] = t1;
    mp[node2] = t2;

    mp[node1]->print();
    mp[node2]->print();
  }
  //
  //
  {
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "======== MyGraph 2 ========" << std::endl;
    std::cout << "THIS IS A REAL EXAMPLE!" << std::endl;
    std::cout << std::endl;

    MyGraph<double> G;
    std::cout << "CONTEXT2 SHOULD NOT HAVE CREATED Tree for nullptr node" << std::endl;
    G.my_ctx().lock()->print();
    //
    G.print();
    //G.entry = cycle_ptr<MyNode>(G.get_ctx(), new MyNode { .val = -1.0 });
    std::cout << "WILL MAKE NODE -1" << std::endl;
    G.entry = G.make_node(-1.0);
    std::cout << "CONTEXT SHOULD HAVE CREATED Tree for -1 node" << std::endl;
    //
    std::cout << "FIRST PRINT2!" << std::endl;
    G.print();
    // make cycle
    auto ptr1 = G.make_node_owned(1.0, G.entry);
    auto ptr2 = G.make_node_owned(2.0, ptr1);
    auto ptr3 = G.make_node_owned(3.0, ptr2);
    // JUST ASSIGN OWNED TO Head again... copy is not really necessary
    auto ptr_head = G.entry.copy_owned(ptr3);
    //

    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    //G.entry.get().neighbors.push_back(ptr1);
    //G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    //ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    //ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    //ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    //
    auto lsptr = G.my_ctx().lock();
    std::cout << "lsptr2 -> " << lsptr << std::endl;
    if (lsptr)
      lsptr->collect();
    //G.my_ctx().lock()->collect();
    std::cout << "FINAL PRINT 2!" << std::endl;
    G.print();
  }

  std::cout << "Next example requires merging trees... aborting (FOR NOW!)" << std::endl;
  return 0;
  //
  {
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "======== MyGraph ========" << std::endl;
    std::cout << std::endl;

    MyGraph<double> G;
    std::cout << "CONTEXT SHOULD NOT HAVE CREATED Tree for nullptr node" << std::endl;
    G.my_ctx().lock()->print();
    //
    G.print();
    //G.entry = cycle_ptr<MyNode>(G.get_ctx(), new MyNode { .val = -1.0 });
    std::cout << "WILL MAKE NODE -1" << std::endl;
    G.entry = G.make_node(-1.0);
    std::cout << "CONTEXT SHOULD HAVE CREATED Tree for -1 node" << std::endl;
    //
    std::cout << "FIRST PRINT!" << std::endl;
    G.print();
    // make cycle
    auto ptr1 = G.make_node(1.0);
    auto ptr2 = G.make_node(2.0);
    auto ptr3 = G.make_node(3.0);
    // -1/HEAD -> 1 -> 2 -> 3 -> (-1/HEAD)
    //
    //G.entry.get().neighbors.push_back(ptr1);
    G.entry.get().neighbors.push_back(ptr1.copy_owned(G.entry));
    ptr1.get().neighbors.push_back(ptr2.copy_owned(ptr1));
    ptr2.get().neighbors.push_back(ptr3.copy_owned(ptr2));
    ptr3.get().neighbors.push_back(G.entry.copy_owned(ptr3));
    //
    auto lsptr = G.my_ctx().lock();
    std::cout << "lsptr -> " << lsptr << std::endl;
    if (lsptr)
      lsptr->collect();
    //G.my_ctx().lock()->collect();
    std::cout << "FINAL PRINT!" << std::endl;
    G.print();
  } // WILL LEAK

  std::cout << "FINISHED!" << std::endl;
  return 0;
}